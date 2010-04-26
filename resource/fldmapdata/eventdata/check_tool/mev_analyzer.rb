#################################################################################
#
# @brief  .mevファイル解析クラス
#
# @file   mev_analyzer.rb
# @author obata
# @date   2010.04.24
#
################################################################################# 
# -*- coding: utf-8 -*-
require "mev_parser.rb"
require "obj_event_section_parser.rb"
require "obj_event_parser.rb"


RESULT_FILENAME = "event_check_result.txt"


# 視線トレーナーイベントのリスト
$event_type_trainer_list = 
[
"EV_TYPE_TR",
"EV_TYPE_TRAINER",
"EV_TYPE_TR_EYEALL",
"EV_TYPE_TRAINER_EYEALL",
"EV_TYPE_TR_KYORO",
"EV_TYPE_TRAINER_KYORO",
"EV_TYPE_TR_SPINS_L",
"EV_TYPE_TR_SPINS_R",
"EV_TYPE_TR_SPINM_L",
"EV_TYPE_TR_SPINM_R",
"EV_TYPE_TRAINER_SPIN_STOP_L",
"EV_TYPE_TRAINER_SPIN_STOP_R",
"EV_TYPE_TRAINER_SPIN_MOVE_L",
"EV_TYPE_TRAINER_SPIN_MOVE_R",
"EV_TYPE_TR_D_ALT",
"EV_TYPE_TRAINER_DASH_ALTER",
"EV_TYPE_TR_D_REA",
"EV_TYPE_TRAINER_DASH_REACT",
"EV_TYPE_TR_D_ACC",
"EV_TYPE_TRAINER_DASH_ACCEL",
"EV_TYPE_TR_D_KYORO",
"EV_TYPE_TRAINER_DASH_ACCEL_KYORO",
"EV_TYPE_TR_D_SPINS_L",
"EV_TYPE_TR_D_SPINS_R",
"EV_TYPE_TR_D_SPINM_L",
"EV_TYPE_TR_D_SPINM_R",
"EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_L",
"EV_TYPE_TRAINER_DASH_ACCEL_SPIN_STOP_R",
"EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_L",
"EV_TYPE_TRAINER_DASH_ACCEL_SPIN_MOVE_R",
]

class MEVAnalyzer

  def initialize
    @out_data = Array.new # 出力データ
    @analyze_flag_trainer = false # 視線トレーナー解析フラグ
  end

  # アクセッサ
  attr_accessor :analyze_flag_trainer

  private
  #--------------------------------------------------
  # OBJイベントが視線トレーナーかどうかをチェックする
  #--------------------------------------------------
  def check_obj_event_is_trainer( event )
    $event_type_trainer_list.include?( event.EVENT_TYPE_Number )
  end

  private
  #------------------------------
  # 解析結果をファイルに出力する
  #------------------------------
  def output_result( filename )
    file = File.open( filename, "w" )
    file.puts( @out_data )
    file.close
  end

  private
  #---------------------------------
  # 出力データにファイル名を追加する
  #---------------------------------
  def output_filename( mev )
    @out_data << "------------------------\r\n"
    @out_data << File.basename( mev.filename ) + "\r\n"
    @out_data << "------------------------\r\n"
  end

  private
  #--------------------------------------------------
  # 出力データに視線トレーナーイベント情報を追加する
  #--------------------------------------------------
  def output_trainer( obj_section )
    obj_section.event.each do |event|
      if check_obj_event_is_trainer( event ) then
        puts event.event_number
        puts event.ID_name
        data = event.OBJ_CODE_Number
        data += ", "
        data += "ID = #{event.ID_name}"
        data += "\r\n"
        @out_data << data
      end
    end
  end

  public
  #----------
  # 解析する
  #----------
  def analyze( mev_files )
    print( "< Analyze Start!! > --> " )

    # すべての.mevファイルを解析する
    mev_files.each do |mev|

      print( "#{File.basename( mev.filename )} --> " )
      STDOUT.flush

      output_filename( mev ) # ファイル名
      if @analyze_flag_trainer then output_trainer( mev.obj_section ) end # 視線トレーナー

      puts( "OK!" )
      STDOUT.flush

    end

    puts( "< Analyze Finish!! >" )

    # 解析結果を出力
    output_result( RESULT_FILENAME )
  end

end
