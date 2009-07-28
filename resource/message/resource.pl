#===============================================================================
# メッセージコンバータ用 メッセージリソース管理パッケージ
# メインのスクリプトでrequireして使う
# 
# GameFreak taya
#
# 2008.8.27
#	・１つのメッセージデータに複数言語タイプを持たせることが出来る構造に対応させた。
#	・使っていなかったグループパラメータへの対応機能を削除（動作高速化，ソース整理のため）
#
#===============================================================================
package resource;

require 'tool.pl';

#===================================================
# Params
#===================================================
my $DataExt = ".dat";

#===================================================
# Globals
#===================================================
my @MsgLen = ();
my @MsgBody = ();
my @AreaWidth = ();
my $MsgCount = 0;


# メッセージエディタのタグ機能でカラー変更する際、
# カラー変更開始タグだけ設定して、終了タグ（元の色に戻す）を設定していないケースがあるので、
# それを自動で対処するためのパラメータ
my $LatestColorChangeParam = 0;

#===================================================
# Consts
#===================================================
use constant EOM_CODE		=> 0xffff;	# EOMコード
use constant CR_CODE		=> 0xfffe;	# 改行コード
use constant TAG_CODE		=> 0xf000;	# タグ開始コード

use constant TAGTYPE_WORDSET_WORD   => 0x01;
use constant TAGTYPE_WORDSET_NUMBER => 0x02;

use constant MSG_HEADER_SIZE		=> 0x000c;	# ファイルヘッダのサイズ
use constant LANGBLOCK_HEADER_SIZE	=> 0x0004;	# 言語ブロックヘッダのサイズ
use constant STR_HEADER_SIZE		=> 0x0008;	# 文字列に付くヘッダのｻｲｽﾞ

#===================================================
# グループごとの初期化
# input 0 : 文字配置順（配置順に文字が並んだutf16-le文字列）
#===================================================
sub init {
	@MsgLen = ();
	@MsgLenTotal = ();
	@MsgBody = ();
	@AreaWidth = ();
	$MsgCount = ();
}
#===============================================================
# メッセージ追加
# return  1で正常終了，0で解析できないエラー
#===============================================================
sub add_msg {
	my $txt = shift;
	my $lang_idx = shift;
	my $org_lang_flag = shift;
	my $area_width = shift;
	my $data = "";
	my $len = "";

	my $i = 0;
	my $str = $txt;
	my $pos_o = index($txt, '[');
	my $pos_c;
	my $tag;
	my $err;

	$LatestColorChangeParam = 0;

	&encode_init();

	while($pos_o >= 0)
	{
		$pos_c = index($txt, ']', $pos_o+1);
		if($pos_c < 0){
			print &tool::enc_sjis("ERR CODE=1\n");
			return 0;
		}
		$str = substr($txt, $i, ($pos_o - $i));
		$tag = substr($txt, $pos_o, ($pos_c-$pos_o+1));

		$data .= &encode_game_strcode($str, \$err);
		if( $err )
		{
			print &tool::enc_sjis("ERR CODE=2\n");
			return 0;
		}
		
		$tag = &tag_bin($tag);
		if($tag eq "")
		{
			print &tool::enc_sjis("ERR CODE=3\n");
			return 0;
		}
		$data .= $tag;

		$i = $pos_c + 1;
		$pos_o = index($txt, '[', $i);
		if($pos_o < 0)
		{
			if($i < length($txt)){
				$str = substr($txt, $i, length($txt) - $i);
			}else{
				$str = "";
			}
			last;
		}
	}

	if($str ne "")
	{
		$data .= &encode_game_strcode($str, \$err);
		if( $err )
		{
			print &tool::enc_sjis("ERR CODE=4\n");
			return 0;
		}
	}

	# デフォルトカラーに戻されていない単語
	if( $LatestColorChangeParam != 0 )
	{
		$data .= &tag_bin('[FF:00:色:0]');
	}

	$data .= pack('S', EOM_CODE);

	# １文字２バイト固定なので
	$len = length($data) / 2;


	# 開発オリジナル言語なら文字列長設定を実際より大きく取っておく
	#（=日本語は文字数が短いため、ローカライズ時のメモリ不足原因にならないよう）
	if( $org_lang_flag )
	{
		my $i;
		for($i=0; $i<($len-1); $i++)
		{
			$data .= pack('S', EOM_CODE);
		}
		$len = $len*2 - 1;
	}


	# 個別文字列長はカンマ区切り文字列でグローバルに保存
	$len .= ",";
	$MsgLen[$lang_idx] .= $len;

	# 文字列本体もグローバルに保存
	$MsgBody[$lang_idx] .= $data;


	if( $lang_idx == 0 )
	{
		$AreaWidth[ $MsgCount ] = $area_width;
		$MsgCount++;
	}

	return 1;
}


#===============================================================
# データ出力（外部インターフェイス）
# input 0: 処理中のファイル名（拡張子を変更したものを出力）
#       1  出力先ディレクトリパス
# return  異常終了時はdie
#===============================================================
sub output {

	my $filename = shift;
	my $dir = shift;
	my $defLangIdx = shift;

#	if(GroupName == 1)
	if( 1 )
	{
		$filename = &tool::change_ext($filename, $DataExt);
		$filename = &tool::get_filename_from_path($filename);
		$filename = $dir . $filename;

		&output_main($filename);
	}
}

#===============================================================
# データ出力（内部メイン）
# input 0: 出力ファイルパス
#       1  対象データインデックス
#       2  同一IDで別文字列がある場合の種類数（ひらがな・漢字など）
# return  異常終了時はdie
#===============================================================
sub output_main {
	my $file = shift;

	my $dirPath = $file;
	$dirPath =~ s/\/[^\/]+$//g;


	if(open (RES, ">$file") )
	{
		my $numLangs;
		my $langBlockOfs;
		my $dat;
		my @len;
		my $val;
		my $rand_seed;
		my $ofs_sum;
		my $remSize;
		my $i;
		my $s;

		binmode(RES);

	# 格納言語数=配列要素数のハズ
		$numLangs = @MsgLen;

	# 2byte : 格納言語数
		$dat = pack('S', $numLangs);
		syswrite (RES, $dat, 2);

	# 2byte : １言語あたり文字列数
		$dat = pack('S', $MsgCount);
		syswrite (RES, $dat, 2);

	# 4byte : 最大サイズの言語ブロックバイト数
		$dat = pack('I', &get_max_langblock_size());
		syswrite (RES, $dat, 4);

	# 4byte : 予約領域
		$dat = pack('I', 0);
		syswrite (RES, $dat, 4);

# 次の２バイトに暗号用乱数
#		$rand_seed = calc_crc($file);
#		$dat = pack('S', $rand_seed);
#		syswrite (RES, $dat, 2);
#		print &tool::enc_sjis("RndSeed = $rand_seed\n");

	# 言語ブロックへのオフセットテーブル（ファイル先頭からのオフセット）
		$langBlockOfs = MSG_HEADER_SIZE + ($numLangs * 4);
		for($i=0; $i<$numLangs; $i++)
		{
			$dat = pack('I', $langBlockOfs);
			syswrite (RES, $dat, 4);
			$langBlockOfs += &calc_langblock_size($i);
		}

	# 言語ブロック本体
		for($i=0; $i<$numLangs; $i++)
		{
			# 最初にブロックのサイズ
			$remSize = &calc_langblock_size($i);

			$dat = pack('I', $remSize);
			syswrite (RES, $dat, 4);
			$remSize -= 4;

			# 各文字列へのオフセットテーブル（１件４バイト）
			@len = split(/,/, $MsgLen[$i]);
			$ofs_sum = 0;
			for($s=0; $s<$MsgCount; $s++)
			{
				# 言語ブロック先頭からのオフセット計算
				$val = LANGBLOCK_HEADER_SIZE + ($MsgCount  * STR_HEADER_SIZE) + $ofs_sum;
				$dat =  pack('I', $val);
				$dat .= pack('S', $len[$s]);
				$dat .= pack('S', $AreaWidth[$s]);
				syswrite (RES, $dat, STR_HEADER_SIZE);
				$remSize -= STR_HEADER_SIZE;
				$ofs_sum += $len[$s] * 2;
			}

			# 最後にメッセージデータ
			syswrite (RES, $MsgBody[$i], length($MsgBody[$i]));
			$remSize -= length($MsgBody[$i]);

			# パディング
			$dat = pack('C', $val);;
			while($remSize > 0)
			{
				syswrite (RES, $dat, 1);
				$remSize--;
			}
		}

		close(RES);

		system("msgenc $file\n");
		print &tool::enc_sjis("-> $file\n");

	}
	else
	{
		die "$file が作成できません dir=$dirPath  !";
	}
}

# ファイル名文字列からCRC計算
sub calc_crc {
	my  $filename = shift;
	my  ($crc, $p, $d, $len, $count);

	$len = length( $filename );
	$p = 0;
	$crc = 0;
	while( $len-- )
	{
		$d = ord(substr($filename, $p++, 1));
		$count = 8;
		while( $count-- )
		{
			$crc ^= 0x8003 if ($crc <<= 1) & 0x10000;
			$crc ^= 1 if ($d<<=1) & 0x100;
		}
	}
	return $crc & 0xffff;
}


#---------------------------------------------------------------------------------------
# これ以下は外部からの呼び出しを想定していない
#---------------------------------------------------------------------------------------
my $EncodeSkipCR_Flag = 0;

sub encode_init {	# source local
	$EncodeSkipCR_Flag = 0;
}
#===============================================================
# ゲームで扱う文字コードに変換
# input 0:	ファイルから読んだままの文字（utf8）
# return  	ゲーム内文字コード
#===============================================================
sub encode_game_strcode {	# source local
	my $str = shift;
	my $err_flag = shift;
	my $i;
	my $letterCnt;
	my $ret = "";
	my $letter;
	my $t;


	$str = &tool::enc_u16($str);
	$letterCnt = length($str) / 2;
	for($i = 0; $i < $letterCnt; $i++)
	{
		$letter = substr($str, $i*2, 2);
		$t = $letter;
		Encode::from_to($t, 'utf16-le', 'utf8');

		if($t eq "\n")
		{
			if( $EncodeSkipCR_Flag == 0 )
			{
				$ret .= pack('S', CR_CODE);
			}
			else
			{
				$EncodeSkipCR_Flag = 0;
			}
		}
		elsif($t eq "▼")
		{
			$ret .= pack('S', TAG_CODE);
			$ret .= pack('S', 0xbe00);
			$EncodeSkipCR_Flag = 1;		# 次の改行は無視する
		}
		elsif($t eq "▽")
		{
			$ret .= pack('S', TAG_CODE);
			$ret .= pack('S', 0xbe01);
			$EncodeSkipCR_Flag = 1;		# 次の改行は無視する
		}
		else
		{
			$ret .= pack('S', ord(substr($letter,0,1)) | (ord(substr($letter,1,1)) << 8));
		}
	}
	$$err_flag = 0;
	return $ret;

# utf16-le そのまま使うならこれだけでよい
#	return &tool::enc_u16($str);
}

#===============================================================
# タグ文字列をバイナリ化
# input 0: タグ文字列
# return バイナリパックされたタグデータ
#===============================================================
sub tag_bin {
	my $tag_str = shift;

	my @elems = split(/[\[\]:]/, $tag_str);

	my $tag_val;
	my $tag_type;
	my $tag_param;
	my $tag_hi;
	my $tag_lo;

	shift @elems;	# 最初に空要素が入ってしまうので

	if (@elems < 3 )
	{
		return "";
	}


	# 最初は開始コード（2byte）
	$tag_val = pack('S', TAG_CODE);

	# 次にタグ種類コード（2byte）
	$tag_hi  = hex($elems[0]);
	$tag_lo  = hex($elems[1]);
	$tag_type = hex($tag_hi) * 256 + hex($tag_lo);
	$tag_val .= pack('S', $tag_type);

	# 次にパラメータ数（2byte）
	my $elem_max = @elems - 3;
	$tag_val .= pack('S', $elem_max);

	# WORDSET用のタグなら必ず引数があるハズ
	if( ($tag_hi == TAGTYPE_WORDSET_WORD) || ($tag_hi == TARGYPE_WORDSET_NUMBER) ){
		if( $elem_max == 0 ){
			return "";
		}
	}

	# パラメータ１つあたり2byte
	if($elem_max > 0)
	{
		for(my $i = 0; $i < $elem_max; $i++)
		{
			$tag_param = int($elems[3+$i]);
			if( $tag_type == 0xff00 )
			{
				$LatestColorChangeParam = $tag_param;
			}
			$tag_val .= pack('S', $tag_param);
		}
	}

	return $tag_val;
}


#===============================================================
# 最大の言語ブロックサイズ（バイト数）を返す
# return 最大の言語ブロックサイズ（バイト）
#===============================================================
sub get_max_langblock_size {
	my $numLangs = @MsgLen;
	my $maxSize = 0;
	my $tmpSize;
	my $i;

	for($i=0; $i<$numLangs; $i++)
	{
		$tmpSize = &calc_langblock_size($i);
		if( $maxSize < $tmpSize )
		{
			$maxSize = $tmpSize;
		}
	}
	return $maxSize;
};

#===============================================================
# 特定の言語ブロックサイズ（バイト数）を返す
# input 0	言語インデックス
# return 言語ブロックサイズ（バイト）
#===============================================================
sub calc_langblock_size {
	my $idx = shift;
	my $size = LANGBLOCK_HEADER_SIZE;	#  言語ブロックヘッダの最低サイズ

	# 文字列パラメータ配列のサイズ分を追加
	$size += (STR_HEADER_SIZE * $MsgCount);

	# 文字列長のトータルサイズを追加
	$size += length( $MsgBody[$idx] );

	while( $size % 4 )
	{
		$size++;
	}

	return $size;
}


return 1;

