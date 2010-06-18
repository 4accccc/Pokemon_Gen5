class CommandData
  def initialize
    @command = nil
    @seq = nil
    @code_no = nil
    @normal_enable = false
    @init_enable = false
    @recover_enable = false
    @zone_change_enable = false
  end
  attr_accessor :command, :seq, :code_no, 
                :normal_enable, :init_enable, 
                :recover_enable, :zone_change_enable
end

class CommandList
  def initialize
    @list = Array.new 
  end

  def get_all_command_data
    return @list
  end

  def add( command_data )
    @list << command_data
  end

  def search_by_command( command )
    @list.each do |command_data|
      if command_data.command == command then
        return command_data
      end
    end
    return nil
  end

  def search_by_seq( seq )
    @list.each do |command_data|
      if command_data.seq == seq then
        return command_data
      end
    end
    return nil
  end
end
