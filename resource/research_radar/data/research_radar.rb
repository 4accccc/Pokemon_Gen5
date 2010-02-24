####################################################################################
#
# ■brief:  調査レーダー データコンバータ
# □file:   research_radar.rb
# □author: obata
# □date:   2010.02.10
#
# □param: ARGV[0] コンバート ルートディレクトリ
# □param: ARGV[1] 回答データ
# □param: ARGV[2] 質問データ
# □param: ARGV[3] 調査項目データ
# □param: ARGV[4] 趣味データ
# □param: ARGV[5] gmmファイルの出力先
# □param: ARGV[6] デバッグデータの出力先
#
####################################################################################
# カレントディレクトリを変更
Dir::chdir( ARGV[0] )

require "tab_separated_file_reader.rb"  # for TabSeparatedFileReader
require "answer.rb"                     # for Answer
require "question.rb"                   # for Question
require "topic.rb"                      # for Topic
require "hobby.rb"                      # for Hobby
require "gmm_generator.rb"              # for GenerateGMM

FILENAME_ANSWER_DATA   = ARGV[1]  # 回答データ
FILENAME_QUESTION_DATA = ARGV[2]  # 質問データ
FILENAME_TOPIC_DATA    = ARGV[3]  # 調査項目データ
FILENAME_HOBBY_DATA    = ARGV[4]  # 趣味データ
GMM_OUTPUT_DEST        = ARGV[5]  # gmmファイルの出力先
DEBUG_OUTPUT_DEST      = ARGV[6]  # デバッグデータの出力先

answers   = Array.new  # 全回答データ
questions = Array.new  # 全質問データ
topics    = Array.new  # 全調査項目データ
hobbies   = Array.new  # 趣味リスト


#===================================================================================
# ■main
# □全回答の取得
#===================================================================================
# 列インデックス
COLUMN_ANSWER_ID              = 0  # 回答ID
COLUMN_ANSWER_ID_LAVEL        = 1  # 回答IDラベル名
COLUMN_ANSWER_STRING_ID_LAVEL = 2  # gmm内ラベル名
COLUMN_ANSWER_JPN             = 3  # 回答文字列
COLUMN_ANSWER_JPN_KANJI       = 4  # 回答文字列
COLUMN_COLOR                  = 5  # 色
# 行インデックス
ROW_FIRST_ANSWER = 1  # 先頭データ行

fileReader = TabSeparatedFileReader.new
fileReader.ReadFile( FILENAME_ANSWER_DATA )
ROW_FIRST_ANSWER.upto( fileReader.GetRowNum - 1 ) do |rowIdx|
  id              = fileReader.GetCell( rowIdx, COLUMN_ANSWER_ID ).to_i
  idLavel         = fileReader.GetCell( rowIdx, COLUMN_ANSWER_ID_LAVEL )
  stringIDLavel   = fileReader.GetCell( rowIdx, COLUMN_ANSWER_STRING_ID_LAVEL )
  stringJPN       = fileReader.GetCell( rowIdx, COLUMN_ANSWER_JPN )
  stringJPN_KANJI = fileReader.GetCell( rowIdx, COLUMN_ANSWER_JPN_KANJI )
  color           = fileReader.GetCell( rowIdx, COLUMN_COLOR )
  answer = Answer.new
  answer.SetID( id, idLavel )
  answer.SetStringIDLavel( stringIDLavel )
  answer.SetString( stringJPN, stringJPN_KANJI )
  answer.SetColor( color )
  answer.OutputDebug( DEBUG_OUTPUT_DEST )
  answers << answer
end


#===================================================================================
# ■main
# □全質問の取得
#===================================================================================
# 列インデックス
COLUMN_QUESTION_ID                = 0   # 質問ID
COLUMN_QUESTION_ID_LAVEL          = 1   # 質問ID (ラベル名)
COLUMN_QUESTION_TITLE_ID_LAVEL    = 2   # gmm内ラベル名
COLUMN_QUESTION_CAPTION_ID_LAVEL  = 3   # gmm内ラベル名
COLUMN_QUESTION_ANSWER_NUM        = 4   # 選択数
COLUMN_QUESTION_BIT_COUNT         = 5   # bit数
COLUMN_QUESTION_TITLE_JPN         = 6   # 回答文字列 (かな)
COLUMN_QUESTION_TITLE_JPN_KANJI   = 7   # 回答文字列 (漢字)
COLUMN_QUESTION_CAPTION_JPN       = 8   # 回答文字列 (かな)
COLUMN_QUESTION_CAPTION_JPN_KANJI = 9   # 回答文字列 (漢字)
COLUMN_QUESTION_ANSWER_ID_0       = 10   # 回答0
COLUMN_QUESTION_ANSWER_ID_1       = 12   # 回答1
COLUMN_QUESTION_ANSWER_ID_2       = 14  # 回答2 
COLUMN_QUESTION_ANSWER_ID_3       = 16  # 回答3 
COLUMN_QUESTION_ANSWER_ID_4       = 18  # 回答4 
COLUMN_QUESTION_ANSWER_ID_5       = 20  # 回答5 
COLUMN_QUESTION_ANSWER_ID_6       = 22  # 回答6 
COLUMN_QUESTION_ANSWER_ID_7       = 24  # 回答7 
COLUMN_QUESTION_ANSWER_ID_8       = 26  # 回答8 
COLUMN_QUESTION_ANSWER_ID_9       = 28  # 回答9 
COLUMN_QUESTION_ANSWER_ID_10      = 30  # 回答10
COLUMN_QUESTION_ANSWER_ID_11      = 32  # 回答11
COLUMN_QUESTION_ANSWER_ID_12      = 34  # 回答12
COLUMN_QUESTION_ANSWER_ID_13      = 36  # 回答13
COLUMN_QUESTION_ANSWER_ID_14      = 38  # 回答14
COLUMN_QUESTION_ANSWER_ID_15      = 40  # 回答15
COLUMN_QUESTION_ANSWER_ID_16      = 42  # 回答16
COLUMN_QUESTION_ANSWER_ID_17      = 44  # 回答17
# 行インデックス
ROW_FIRST_QUESTION = 1  # 先頭データ行

fileReader = TabSeparatedFileReader.new
fileReader.ReadFile( FILENAME_QUESTION_DATA )
ROW_FIRST_QUESTION.upto( fileReader.GetRowNum - 1 ) do |rowIdx|
  id               = fileReader.GetCell( rowIdx, COLUMN_QUESTION_ID ).to_i
  idLavel          = fileReader.GetCell( rowIdx, COLUMN_QUESTION_ID_LAVEL )
  titleIDLavel     = fileReader.GetCell( rowIdx, COLUMN_QUESTION_TITLE_ID_LAVEL )
  captionIDLavel   = fileReader.GetCell( rowIdx, COLUMN_QUESTION_CAPTION_ID_LAVEL )
  answerNum        = fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_NUM ).to_i
  bitCount         = fileReader.GetCell( rowIdx, COLUMN_QUESTION_BIT_COUNT ).to_i
  titleJPN         = fileReader.GetCell( rowIdx, COLUMN_QUESTION_TITLE_JPN )
  titleJPN_KANJI   = fileReader.GetCell( rowIdx, COLUMN_QUESTION_TITLE_JPN_KANJI )
  captionJPN       = fileReader.GetCell( rowIdx, COLUMN_QUESTION_CAPTION_JPN )
  captionJPN_KANJI = fileReader.GetCell( rowIdx, COLUMN_QUESTION_CAPTION_JPN_KANJI )
  answerID = Array.new
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_0  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_1  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_2  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_3  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_4  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_5  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_6  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_7  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_8  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_9  ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_10 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_11 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_12 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_13 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_14 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_15 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_16 ).to_i
  answerID << fileReader.GetCell( rowIdx, COLUMN_QUESTION_ANSWER_ID_17 ).to_i
  question  = Question.new
  question.SetID( id, idLavel )
  question.SetStringIDLavel( titleIDLavel, captionIDLavel )
  question.SetTitleString( titleJPN, titleJPN_KANJI )
  question.SetCaptionString( captionJPN, captionJPN_KANJI )
  question.SetAnswerNum( answerNum )
  question.SetBitCount( bitCount )
  answerID.each do |ansID|
    question.AddAnswer( answers[ ansID ] )
  end
  question.OutputDebug( DEBUG_OUTPUT_DEST )
  questions << question
end


#===================================================================================
# ■main
# □全調査項目の取得
#===================================================================================
# 列インデックス
COLUMN_TOPIC_ID                      = 0   # 調査ID
COLUMN_TOPIC_ID_LAVEL                = 1   # 調査ID (ラベル名)
COLUMN_TOPIC_TITLE_STRING_ID_LAVEL   = 2   # gmm内ラベル名 (題名)
COLUMN_TOPIC_CAPTION_STRING_ID_LAVEL = 3   # gmm内ラベル名 (補足)
COLUMN_TOPIC_TITLE_JPN               = 4   # 題名 (かな)
COLUMN_TOPIC_TITLE_JPN_KANJI         = 5   # 題名 (漢字)
COLUMN_TOPIC_CAPTION_JPN             = 6   # 補足 (かな)
COLUMN_TOPIC_CAPTION_JPN_KANJI       = 7   # 補足 (漢字)
COLUMN_TOPIC_QUESTION_ID_0           = 8   # 質問0
COLUMN_TOPIC_QUESTION_ID_1           = 10  # 質問1
COLUMN_TOPIC_QUESTION_ID_2           = 12  # 質問2 
# 行インデックス
ROW_FIRST_TOPIC = 1  # 先頭データ行

fileReader = TabSeparatedFileReader.new
fileReader.ReadFile( FILENAME_TOPIC_DATA ) 
ROW_FIRST_TOPIC.upto( fileReader.GetRowNum - 1 ) do |rowIdx|
  id                    = fileReader.GetCell( rowIdx, COLUMN_TOPIC_ID ).to_i
  idLavel               = fileReader.GetCell( rowIdx, COLUMN_TOPIC_ID_LAVEL )
  titleStringID_lavel   = fileReader.GetCell( rowIdx, COLUMN_TOPIC_TITLE_STRING_ID_LAVEL )
  captionStringID_lavel = fileReader.GetCell( rowIdx, COLUMN_TOPIC_CAPTION_STRING_ID_LAVEL )
  titleJPN              = fileReader.GetCell( rowIdx, COLUMN_TOPIC_TITLE_JPN )
  titleJPN_KANJI        = fileReader.GetCell( rowIdx, COLUMN_TOPIC_TITLE_JPN_KANJI )
  captionJPN            = fileReader.GetCell( rowIdx, COLUMN_TOPIC_CAPTION_JPN )
  captionJPN_KANJI      = fileReader.GetCell( rowIdx, COLUMN_TOPIC_CAPTION_JPN_KANJI )
  questionID  = Array.new
  questionID << fileReader.GetCell( rowIdx, COLUMN_TOPIC_QUESTION_ID_0  ).to_i
  questionID << fileReader.GetCell( rowIdx, COLUMN_TOPIC_QUESTION_ID_1  ).to_i
  questionID << fileReader.GetCell( rowIdx, COLUMN_TOPIC_QUESTION_ID_2  ).to_i
  topic  = Topic.new
  topic.SetID( id, idLavel )
  topic.SetStringIDLavel( titleStringID_lavel, captionStringID_lavel )
  topic.SetTitle( titleJPN, titleJPN_KANJI )
  topic.SetCaption( captionJPN, captionJPN_KANJI )
  questionID.each do |qID|
    topic.AddQuestion( questions[ qID ] )
  end
  topic.OutputDebug( DEBUG_OUTPUT_DEST )
  topics << topic
end


#===================================================================================
# ■main
# □趣味リストの取得
#=================================================================================== 
# 列インデックス
COLUMN_HOBBY_ID        = 0  # 趣味ID
COLUMN_HOBBY_ID_LAVEL  = 1  # 趣味IOラベル名
COLUMN_HOBBY_JPN       = 3  # 趣味文字列 (かな)
COLUMN_HOBBY_JPN_KANJI = 4  # 趣味文字列 (漢字)
# 行インデックス
ROW_FIRST_HOBBY = 2  # 先頭データ

# 全ての文字列を取得
fileReader = TabSeparatedFileReader.new
fileReader.ReadFile( FILENAME_HOBBY_DATA )
ROW_FIRST_HOBBY.upto( fileReader.GetRowNum - 1 ) do |rowIdx|
  id              = fileReader.GetCell( rowIdx, COLUMN_HOBBY_ID ).to_i
  idLavel         = fileReader.GetCell( rowIdx, COLUMN_HOBBY_ID_LAVEL ).strip
  stringJPN       = fileReader.GetCell( rowIdx, COLUMN_HOBBY_JPN ).strip
  stringJPN_KANJI = fileReader.GetCell( rowIdx, COLUMN_HOBBY_JPN_KANJI ).strip
  hobby = Hobby.new
  hobby.SetID( id, idLavel )
  hobby.SetString( stringJPN, stringJPN_KANJI )
  hobby.OutputDebug( DEBUG_OUTPUT_DEST )
  hobbies << hobby
end


#===================================================================================
# ■main
# □アンケート回答インデックス データ出力
#===================================================================================
answerNum       = Array.new  # 項目数
answerBitCount  = Array.new  # bit数
answerNumOffset = Array.new  # 項目数オフセット
answerBitOffset = Array.new  # bit数オフセット

# データ集計
answerNum[0]       = questions[0].answerNum
answerBitCount[0]  = questions[0].bitCount
answerNumOffset[0] = 0
answerBitOffset[0] = 0

1.upto( questions.size - 1 ) do |questionIdx|
  answerNum      [ questionIdx ] = questions[ questionIdx ].answerNum
  answerBitCount [ questionIdx ] = questions[ questionIdx ].bitCount
  answerNumOffset[ questionIdx ] = answerNumOffset[ questionIdx - 1 ] + ( questions[ questionIdx - 1 ].answerNum - 1 ) # "無回答"を除くため1を引く
  answerBitOffset[ questionIdx ] = answerBitOffset[ questionIdx - 1 ] + questions[ questionIdx - 1 ].bitCount
end

# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const QUESTIONNAIRE_INDEX QuestionnaireIndex[] = "
outData << "{"
0.upto( questions.size - 1 ) do |qIdx|
  outData << "  { #{answerNum[qIdx]}, #{answerBitCount[qIdx]}, #{answerNumOffset[qIdx]}, #{answerBitOffset[qIdx]} },"
end
outData << "};"

# 出力
file = File.open( "questionnaire_index.cdat", "w" )
file.puts( outData )
file.close


#===================================================================================
# ■main
# □gmm 出力 ( questionnaire.gmm )
#===================================================================================
directory       = GMM_OUTPUT_DEST
fileName        = "questionnaire.gmm"
stringLavel     = Array.new
stringJPN       = Array.new
stringJPN_KANJI = Array.new

# 調査項目 (タイトル)
topics.each do |topic|
  stringLavel     << topic.titleStringID_lavel
  stringJPN       << topic.titleJPN
  stringJPN_KANJI << topic.titleJPN_KANJI
end

# 調査項目 (補足)
topics.each do |topic|
  stringLavel     << topic.captionStringID_lavel
  stringJPN       << topic.captionJPN
  stringJPN_KANJI << topic.captionJPN_KANJI
end

# 質問
questions.each do |question|
  stringLavel     << question.titleStringID_lavel
  stringJPN       << question.titleStringJPN
  stringJPN_KANJI << question.titleStringJPN_KANJI
end

# 質問 (補足)
questions.each do |question|
  stringLavel     << question.captionStringID_lavel
  stringJPN       << question.captionStringJPN
  stringJPN_KANJI << question.captionStringJPN_KANJI
end

# 回答
answers.each do |answer|
  stringLavel     << answer.stringID_lavel
  stringJPN       << answer.stringJPN
  stringJPN_KANJI << answer.stringJPN_KANJI
end

# gmm 生成
GenerateGMM( directory, fileName, stringLavel, stringJPN, stringJPN_KANJI )


#===================================================================================
# ■main
# □gmm 出力 ( hobby.gmm )
#=================================================================================== 
directory       = GMM_OUTPUT_DEST
fileName        = "hobby.gmm"
stringLavel     = Array.new
stringJPN       = Array.new
stringJPN_KANJI = Array.new

# 趣味
hobbies.each do |hobby|
  stringLavel     << "hobby_%d" % hobby.ID
  stringJPN       << hobby.stringJPN
  stringJPN_KANJI << hobby.stringJPN_KANJI
end

# gmm 生成
GenerateGMM( directory, fileName, stringLavel, stringJPN, stringJPN_KANJI )


#=================================================================================== 
# ■main
# □趣味ID ファイル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
hobbies.each do |hobby|
  outData << "#define #{hobby.ID_lavel} (#{hobby.ID}) //「#{hobby.stringJPN}」"
end
outData << "#define HOBBY_ID_NUM   (#{hobbies.size})   // 趣味IDの数"
outData << "#define HOBBY_ID_MAX   (HOBBY_ID_NUM - 1)  // 趣味IDの最大値"
outData << "#define HOBBY_ID_DUMMY (0xff)              // 趣味IDのダミー値"

# 出力
file = File.open( "hobby_id.h", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □回答ID ファイル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
answers.each do |answer|
  outData << "#define #{answer.ID_lavel} (#{answer.ID}) //「#{answer.stringJPN}」"
end
outData << "#define ANSWER_ID_NUM   (#{answers.size})    // 回答IDの数"
outData << "#define ANSWER_ID_MAX   (ANSWER_ID_NUM - 1)  // 回答IDの最大値"
outData << "#define ANSWER_ID_DUMMY (0xffff)             // 回答IDのダミー値"

# 出力
file = File.open( "answer_id.h", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □質問ID ファイル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
questions.each do |question|
  outData << "#define #{question.ID_lavel} (#{question.ID}) //「#{question.titleStringJPN}」"
end
outData << "#define QUESTION_ID_NUM   (#{questions.size})    // 質問IDの数"
outData << "#define QUESTION_ID_MAX   (QUESTION_ID_NUM - 1)  // 質問IDの最大値"
outData << "#define QUESTION_ID_DUMMY (0xff)                 // 質問IDのダミー値"

# 出力
file = File.open( "question_id.h", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □調査項目ID ファイル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
topics.each do |topic|
  outData << "#define #{topic.ID_lavel} (#{topic.ID}) //「#{topic.titleJPN}」"
end
outData << "#define TOPIC_ID_NUM   (#{topics.size})    // 調査項目IDの数"
outData << "#define TOPIC_ID_MAX   (TOPIC_ID_NUM - 1)  // 調査項目IDの最大値"
outData << "#define TOPIC_ID_DUMMY (0xff)              // 調査項目IDのダミー値"

# 出力
file = File.open( "topic_id.h", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □回答ID ⇒ 文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 StringID_answer[ ANSWER_ID_NUM ] = "
outData << "{"
answers.each do |answer|
  outData << "  #{answer.stringID_lavel},  // [#{answer.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "string_id_answer.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □回答ID ⇒ 表示カラー(R) テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u8 ColorR_answer[ ANSWER_ID_NUM ] = "
outData << "{"
answers.each do |answer|
  outData << "  #{answer.colorR},  // [#{answer.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "color_r_answer.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □回答ID ⇒ 表示カラー(G) テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u8 ColorG_answer[ ANSWER_ID_NUM ] = "
outData << "{"
answers.each do |answer|
  outData << "  #{answer.colorG},  // [#{answer.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "color_g_answer.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □回答ID ⇒ 表示カラー(B) テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u8 ColorB_answer[ ANSWER_ID_NUM ] = "
outData << "{"
answers.each do |answer|
  outData << "  #{answer.colorB},  // [#{answer.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "color_b_answer.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □質問ID ⇒ 文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 StringID_question[ QUESTION_ID_NUM ] = "
outData << "{"
questions.each do |question|
  outData << "  #{question.titleStringID_lavel},  // [#{question.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "string_id_question.cdat", "w" )
file.puts( outData )
file.close

#=================================================================================== 
# ■main
# □質問ID ⇒ 補足文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 CaptionStringID_question[ QUESTION_ID_NUM ] = "
outData << "{"
questions.each do |question|
  outData << "  #{question.captionStringID_lavel},  // [#{question.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "caption_string_id_question.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □質問ID ⇒ 文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u8 AnswerNum_question[ QUESTION_ID_NUM ] = "
outData << "{"
questions.each do |question|
  outData << "  #{question.answers.size},  // [#{question.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "answer_num_question.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □質問ID ⇒ 回答ID テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 AnswerID_question[ QUESTION_ID_NUM ][17] = "
outData << "{"
questions.each do |question|
  data = String.new
  data += "  { "
  0.upto( 16 ) do |answerIdx|
    answer = question.answers[ answerIdx ]
    if answer == nil then 
      data += "0, "
    else 
      data += "#{answer.ID}, "
    end
  end
  data += "},"
  outData << data
end
outData << "};"

# 出力
file = File.open( "answer_id_question.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □調査項目ID ⇒ 題名文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 StringID_topicTitle[ TOPIC_ID_NUM ] = "
outData << "{"
topics.each do |topic|
  outData << "  #{topic.titleStringID_lavel},  // [#{topic.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "string_id_topic_title.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □調査項目ID ⇒ 補足文字列 テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u32 StringID_topicCaption[ TOPIC_ID_NUM ] = "
outData << "{"
topics.each do |topic|
  outData << "  #{topic.captionStringID_lavel},  // [#{topic.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "string_id_topic_caption.cdat", "w" )
file.puts( outData )
file.close


#=================================================================================== 
# ■main
# □調査項目ID ⇒ 質問ID テーブル出力
#===================================================================================
# 出力データ作成
outData = Array.new
outData << "// コンバータにより生成"
outData << "static const u8 Question1_topic[ TOPIC_ID_NUM ] = "
outData << "{"
topics.each do |topic|
  question1 = topic.questions[0]
  outData << "  #{question1.ID_lavel},  // [#{topic.ID_lavel}]"
end
outData << "};"

outData << "static const u8 Question2_topic[ TOPIC_ID_NUM ] = "
outData << "{"
topics.each do |topic|
  question2 = topic.questions[1]
  outData << "  #{question2.ID_lavel},  // [#{topic.ID_lavel}]"
end
outData << "};"

outData << "static const u8 Question3_topic[ TOPIC_ID_NUM ] = "
outData << "{"
topics.each do |topic|
  question3 = topic.questions[2]
  outData << "  #{question3.ID_lavel},  // [#{topic.ID_lavel}]"
end
outData << "};"

# 出力
file = File.open( "question_id_topic.cdat", "w" )
file.puts( outData )
file.close
