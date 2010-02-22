####################################################################################
#
# ■brief:  質問オブジェクト定義
# □file:   answer.rb
# □author: obata
# □date:   2010.02.12
#
####################################################################################


#===================================================================================
# ■質問
#===================================================================================
class Question

  def initialize
    @ID                     = 0          # 質問ID
    @ID_lavel               = nil        # 質問ID (ラベル名)
    @titleStringID_lavel    = nil        # gmm内ラベル名 (質問文字列)
    @titleStringJPN         = nil        # 質問文字列 (かな)
    @titleStringJPN_KANJI   = nil        # 質問文字列 (漢字)
    @captionStringID_lavel  = nil        # gmm内ラベル名 (補足文字列)
    @captionStringJPN       = nil        # 補足文字列 (かな)
    @captionStringJPN_KANJI = nil        # 補足文字列 (漢字)
    @answerNum              = 0          # 回答選択肢の数
    @bitCount               = 0          # 回答を表現するために必要なビット数
    @answers                = Array.new  # 回答配列
  end

  attr_reader :ID, :ID_lavel, 
              :titleStringID_lavel, :titleStringJPN, :titleStringJPN_KANJI, 
              :captionStringID_lavel, :captionStringJPN, :captionStringJPN_KANJI, 
              :answerNum, :bitCount, :answers

  #----------------------------------
  # □brief: 質問IDを設定する
  # □param: id    質問ID
  # □param: lavel 質問ID (ラベル名)
  def SetID( id, lavel )
    @ID       = id
    @ID_lavel = lavel
  end

  #----------------------------------
  # □brief: gmm内ラベル名を設定する
  # □param: title
  # □param: caption
  def SetStringIDLavel( title, caption )
    @titleStringID_lavel = title
    @captionStringID_lavel = caption
  end

  #-------------------------------------------------
  # □brief: 質問文字列を設定する
  # □param: titleStringJPN       質問文字列 (かな)
  # □param: titleStringJPN_KANJI 質問文字列 (漢字)
  def SetTitleString( titleStringJPN, titleStringJPN_KANJI )
    @titleStringJPN       = titleStringJPN
    @titleStringJPN_KANJI = titleStringJPN_KANJI
  end

  #-------------------------------------------------
  # □brief: 補足文字列を設定する
  # □param: captionStringJPN       補足文字列 (かな)
  # □param: captionStringJPN_KANJI 補足文字列 (漢字)
  def SetCaptionString( captionStringJPN, captionStringJPN_KANJI )
    @captionStringJPN       = captionStringJPN
    @captionStringJPN_KANJI = captionStringJPN_KANJI
  end

  #-----------------------------------
  # □brief: 回答選択肢の数を設定する
  # □param: answerNum 選択肢の数
  def SetAnswerNum( answerNum )
    @answerNum = answerNum
  end

  #-------------------------------------------------------
  # □brief: 回答を表現するために必要なビット数を設定する
  # □param: bitCount ビット数
  def SetBitCount( bitCount )
    @bitCount = bitCount
  end

  #--------------------------
  # □brief: 回答を追加する
  # □param: answer 回答
  def AddAnswer( answer )
    if answer.ID != 0 then @answers << answer end # 無回答は保持しない
  end

  #---------------------------------------
  # □brief: デバッグ情報を出力する
  # □param: directory 出力先ディレクトリ
  def OutputDebug( directory )
    # 出力データ作成
    outData = Array.new
    outData << "ID                     = #{@ID}"
    outData << "ID_lavel               = #{@ID_lavel}"
    outData << "titleStringID_lavel    = #{@titleStringID_lavel}"
    outData << "titleStringJPN         = #{@titleStringJPN}"
    outData << "titleStringJPN_KANJI   = #{@titleStringJPN_KANJI}"
    outData << "captionStringID_lavel  = #{@captionStringID_lavel}"
    outData << "captionStringJPN       = #{@captionStringJPN}"
    outData << "captionStringJPN_KANJI = #{@captionStringJPN_KANJI}"
    outData << "answerNum              = #{@answerNum}"
    outData << "bitCount               = #{@bitCount}"
    @answers.each do |answer|
      outData << "A.#{answer.stringJPN} (#{answer.stringJPN_KANJI})"
    end
    # 出力
    fileName = directory + "/question_#{@ID}.txt"
    file = File.open( fileName, "w" )
    file.puts( outData )
    file.close
  end

end
