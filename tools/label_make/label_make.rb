#! ruby -Ks
#=====================================================================================
#
# ローマ字変換ラベル生成スクリプト
#
#=====================================================================================

class LabelMake

  DIC_SRC = 0
  DIC_DST = 1
  U_FLAG = 1
  F_FLAG = 2
  J_FLAG = 3

  def initialize
    @dic = []
    cnt = 0
    open( File.dirname(__FILE__) + "/label.dic" ) {|fp_r|
      while str = fp_r.gets
        @dic[ cnt ] = str
        cnt += 1
      end
    }
  end

  #===========================================================================
  # 与えられた文字列からラベルを生成
  # @param[in]  head_str  ラベルの頭につける文字列（ポケモン名ならMONSNO_とか）
  # @param[in]  str       ラベル変換する文字列
  #
  # @retval ラベル変換した文字列
  #===========================================================================
  def make_label( head_str, str )
    label_str = head_str
    cnt = 0
    bou_str = ""
    mini_flag = 0
    mini_flag_tmp = 0
    tu_flag = 0

    while str[ cnt ] != nil
      moji = str[ cnt + 0 .. cnt + 1 ]
      
      @dic.size.times {|dic_cnt|
        split_dic = @dic[ dic_cnt ].split(/\s+/)
        if moji == split_dic[ DIC_SRC ]
          case split_dic[ DIC_DST ][ 0 ].chr
          when "@"
            if tu_flag == 0
              mini_flag = ( split_dic[ DIC_DST ][ 1 ].chr ).to_i
              label_str << split_dic[ DIC_DST ][ 2 .. split_dic[ DIC_DST ].size - 1 ]
            else
              tu_flag = 0
              mini_flag = ( split_dic[ DIC_DST ][ 1 ].chr ).to_i
              label_str << split_dic[ DIC_DST ][ 2 ].chr
              label_str << split_dic[ DIC_DST ][ 2 .. split_dic[ DIC_DST ].size - 1 ]
            end
          when "-"
            label_str << bou_str
          when "/"
            tu_flag = 1
          when ":"
            if split_dic[ DIC_DST ][ 1 ].chr == "H"
              case mini_flag
              when U_FLAG
                label_str[ label_str.size - 1 ] = "W"
                label_str << split_dic[ DIC_DST ][ 1 .. split_dic[ DIC_DST ].size - 1 ]
              when F_FLAG
                label_str[ label_str.size - 2 ] = "F"
                label_str[ label_str.size - 1 ] = split_dic[ DIC_DST ][ 2 .. split_dic[ DIC_DST ].size - 1 ]
              when J_FLAG
                label_str[ label_str.size - 2 ] = "J"
                label_str[ label_str.size - 1 ] = split_dic[ DIC_DST ][ 2 .. split_dic[ DIC_DST ].size - 1 ]
              else
                label_str[ label_str.size - 1 ] = split_dic[ DIC_DST ][ 1 .. split_dic[ DIC_DST ].size - 1 ]
              end
            else
              label_str[ label_str.size - 1 ] = split_dic[ DIC_DST ][ 1 .. split_dic[ DIC_DST ].size - 1 ]
            end
          else
            if tu_flag == 0
              label_str << split_dic[ DIC_DST ]
            else
              tu_flag = 0
              label_str << split_dic[ DIC_DST ][ 0 ].chr
              label_str << split_dic[ DIC_DST ]
            end
          end
          bou_str = split_dic[ DIC_DST ][ split_dic[ DIC_DST ].size - 1].chr
          if mini_flag_tmp == 0
            mini_flag_tmp = mini_flag
          else
            mini_flag = 0
            mini_flag_tmp = 0
          end
        end
      }
      cnt += 2
    end

    return label_str
  end

end

#使い方サンプル
# require 'label_make'    #パス指定は任意でおこなってください
# include LabelMake
#
# label = LabelMake.new
# p label.make_label( "MONSNO_", "コラッタ" )
#
# MONSNO_KORATTAと表示されます
