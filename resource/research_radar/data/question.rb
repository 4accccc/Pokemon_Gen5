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
    @ID              = 0          # 質問ID
    @stringJPN       = nil        # 質問文字列 (かな)
    @stringJPN_KANJI = nil        # 質問文字列 (漢字)
    @answerNum       = 0          # 回答選択肢の数
    @bitCount        = 0          # 回答を表現するために必要なビット数
    @answers         = Array.new  # 回答配列
  end

  attr_reader :ID, :stringJPN, :stringJPN_KANJI, :answerNum, :bitCount, :answears

  #----------------------------
  # □brief: 質問IDを設定する
  # □param: id 質問ID
  def SetID( id )
    @ID = id
  end

  #-------------------------------------------
  # □brief: 質問文字列を設定する
  # □param: stringJPN       質問文字列 (かな)
  # □param: stringJPN_KANJI 質問文字列 (漢字)
  def SetString( stringJPN, stringJPN_KANJI )
    @stringJPN       = stringJPN
    @stringJPN_KANJI = stringJPN_KANJI
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
    @answers << answer
  end

  #---------------------------------------
  # □brief: デバッグ情報を出力する
  # □param: directory 出力先ディレクトリ
  def OutputDebug( directory )
    # 出力データ作成
    outData = Array.new
    outData << "ID              = #{@ID}"
    outData << "stringJPN       = #{@stringJPN}"
    outData << "stringJPN_KANJI = #{@stringJPN_KANJI}"
    outData << "answerNum       = #{@answerNum}"
    outData << "bitCount        = #{@bitCount}"
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
