#-------------------------------------------------------------------------------
# デバッグサウンド用gmmファイルを作るツール
# 
# 実行すると「sound_data.sadl」の中から「SEQ_」がマッチする文字列を取り出して
# サウンドデバッグ画面で使うgmmファイルを作成します。
# 同じフォルダに「sound_data.sadl」+ messageフォルダーのtemplate.gmmを固定参照します
# ruby gmm_make.rb template.gmm wb_sound_data.sadl soundtestname.gmm
# 2008.01.10 by AkitoMori
# 2009.01.28 k.ohno
#-------------------------------------------------------------------------------


require "nkf"
require "../../tools/gmmconv.rb"





class SEQNAMEConv < GmmRead
  
  def initialize
    super
  end
  
    ### ROWタグ文字列
  
  
  

  ##SEQからROWデータを作る
  def SeqRowAdd()
    gmmno = 0
    @ConvFileLineTmp = @TmplateFileLine
    #サウンドシーケンスのみ取り出し整形してgmmに出力
    @ConvFileLine.each{ |line|
    
      divline = line.split(nil)
      seqname = divline[1]
      
      #「SEQ_」がついている定義名のみ取り出す
      if seqname=~/^SEQ_/ then
        gmmend = divline[2].to_i
        gmmend = gmmend - 1
        for xn in gmmno..gmmend
          
tbufdmy = <<"EOFZ"
          <row id="msg_seq_#{xn}">
          <language name="JPN" width="256">dmy</language>
          <language name="JPN_KANJI" width="256">dmy</language>
          </row>
EOFZ
          tbufdmy = NKF.nkf("-w",tbufdmy)
          @ConvFileLineTmp.push(tbufdmy)
          
        end
        gmmno = divline[2].to_i + 1
        #出力用に名前を加工
        seqidname   = "msg_"+seqname.downcase
        msgstr  = seqname.sub(/SEQ_SE_/,"")
        msgstr  = msgstr.sub(/SEQ_/,"")
        seqmsg  = msgstr.gsub(/_/,"-")
        
 tbuf = <<"EOF"
	<row id="#{seqidname}">
		<language name="JPN" width="256">#{seqmsg}</language>
		<language name="JPN_KANJI" width="256">#{seqmsg}</language>
	</row>
EOF
        tbuf = NKF.nkf("-w",tbuf)
        @ConvFileLineTmp.push(tbuf)
			
      end
    }
    @ConvFileLine = @ConvFileLineTmp
    @ConvFileLineTmp = Array.new
    end
  
end



### メイン
begin
  
  GRead = SEQNAMEConv.new
  GRead.FileRead( ARGV[1] )  ##コンバート対象ファイルを読み込む
  GRead.TmplateRead( ARGV[0] ) ## テンプレを読み込む
  GRead.TmplateMake()  ##  テンプレだけ抜き出す
  GRead.SeqRowAdd() ##SEQをrowタグにして追加
  
  GRead.TmplateRead( ARGV[0] ) ## テンプレを読み込む
  
  GRead.TmplateFootMake()  ##  テンプレの最後を追加
  
  GRead.FileWrite( ARGV[2] )  ##コンバート結果を出力する
      
 ## exit(0)
end
