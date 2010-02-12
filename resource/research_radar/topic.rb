####################################################################################
#
# ■brief:  調査項目オブジェクト定義
# □file:   topic.rb
# □author: obata
# □date:   2010.02.12
#
####################################################################################


#===================================================================================
# ■調査項目
#===================================================================================
class Topic
  
  def initialize
    @ID               = 0          # 項目ID
    @titleJPN         = nil        # 項目名 (かな)
    @titleJPN_KANJI   = nil        # 項目名 (漢字)
    @captionJPN       = nil        # 補足文 (かな)
    @captionJPN_KANJI = nil        # 補足文 (漢字)
    @questions        = Array.new  # 質問配列
  end

  attr_reader :ID, :titleJPN, :titleJPN_KANJI, :captionJPN, :captionJPN_KANJI, :questions

  #----------------------------
  # □brief: 調査IDを設定する
  # □param: id 調査ID
  def SetID( id )
    @ID = id
  end

  #--------------------------------------
  # □brief: 項目名を設定する
  # □param: titleJPN       項目名 (かな)
  # □param: titleJPN_KANJI 項目名 (漢字)
  def SetTitle( titleJPN, titleJPN_KANJI )
    @titleJPN       = titleJPN
    @titleJPN_KANJI = titleJPN_KANJI
  end

  #----------------------------------------
  # □brief: 補足文を設定する
  # □param: captionJPN       補足文 (かな)
  # □param: captionJPN_KANJI 補足文 (漢字)
  def SetCaption( captionJPN, captionJPN_KANJI )
    @captionJPN       = captionJPN
    @captionJPN_KANJI = captionJPN_KANJI
  end

  #-------------------------
  # □brief: 質問を追加する
  # □param: question 質問
  def AddQuestion( question )
    @questions << question
  end

  #---------------------------------------
  # □brief: デバッグ情報を出力する
  # □param: directory 出力先ディレクトリ
  def OutputDebug( directory )
    # 出力データ作成
    outData = Array.new
    outData << "ID               = #{@ID}"
    outData << "titleJPN         = #{@titleJPN}"
    outData << "titleJPN_KANJI   = #{@titleJPN_KANJI}"
    outData << "captionJPN       = #{@captionJPN}"
    outData << "captionJPN_KANJI = #{@captionJPN_KANJI}"
    @questions.each do |question|
      outData << "Q.#{question.stringJPN} (#{question.stringJPN_KANJI})"
    end
    # 出力
    fileName = directory + "/topic_#{@ID}.txt"
    file = File.open( fileName, "w" )
    file.puts( outData )
    file.close
  end

end 
