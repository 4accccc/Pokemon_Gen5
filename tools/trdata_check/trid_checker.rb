require "trid_check_result.rb"


BTL_SET_COMMAND_LIST = [
  "_TRAINER_BTL_SET",
  "_TRAINER_BTL_SP_SET",
  "_TRAINER_MULTI_BTL_SET",
  "_TRADE_AFTER_TRAINER_BTL_SET",
]


class TrIDChecker
  def initialize
    @result = TrIDCheckResult.new
    @check_files = Array.new
    @trid_list = Array.new
  end

  attr_reader :result, :trid_list

  def add_check_files( target_dir )
    @check_files += Dir[ target_dir ]
  end

  def get_filelines( filename )
    file = File.open( filename, "r" )
    lines = file.readlines
    file.close 
    return lines
  end

  def setup_trid_list( path_to_trid_def_file )
    lines = self.get_filelines( path_to_trid_def_file )
    lines.each do |line|
      line.strip!
      if line =~ /#define\s+(TRID_\w*)/ then 
        @trid_list << $1
      end
    end
    @trid_list.delete( "TRID_NULL" )
    @trid_list.delete( "TRID_MAX" )
  end

  def analyze
    @check_files.each do |filename|
      search_trid_in_file( filename )
    end
  end

  def search_trid_in_file( filename )
    lines = self.get_filelines( filename )
    lines.each do |line|
      self.search_trid_in_line( filename, line )
    end
  end

  def search_trid_in_line( filename, line )
    while self.check_line_has_trid( line )
      trid = self.get_trid_in_line( line )
      if self.check_trid_is_valid( filename, line, trid ) then
        @result.add( trid, "\t#{filename.strip}: #{line.strip}\r\n" )
      end
      line.gsub!( trid, "" )
    end
  end

  def check_line_has_trid( line )
    return line.include?( "TRID_" )
  end

  def get_trid_in_line( line )
    trid = nil
    if line =~ /(TRID_\w*)/ then
      trid = $1
    end
    return trid
  end

  def check_trid_is_valid( filename, line, trid )
    if filename.include?( ".mev" ) then
      return true
    elsif filename.include?( ".ev" ) then
      return self.check_trid_is_valid_in_ev( line, trid )
    end
  end

  def check_trid_is_valid_in_ev( line, trid )
    if self.check_trid_commentout( line, trid ) then
      return false
    end
    if self.check_trid_in_btlset_command( line, trid ) == false
      return false
    end
    return true
  end

  def check_trid_commentout( line, trid )
    if line.include?( "//" ) then
      if line.index( "//" ) < line.index( trid ) then
        return true # コメントアウトされている
      else
        return false # コメントアウトされていない
      end
    end
    return false
  end

  def check_trid_in_btlset_command( line, trid )
    BTL_SET_COMMAND_LIST.each do |command|
      if line.include?( command ) then
        return true
      end
    end 
    return false
  end
end
