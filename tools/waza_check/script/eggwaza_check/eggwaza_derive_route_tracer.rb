require "eggwaza_checker.rb"
require "eggwaza_derive_route.rb"


# タマゴ技の継承ルート探索機
class EggWazaDeriveRouteTracer

  def initialize
    @root_mons_name = nil
    @target_waza_name = nil
    @current_route = Array.new
    @next_route = Array.new
    @route_found_flag = false
    @vaild_route = nil
    @checked_mons_list = Array.new
  end

  def init_trace( mons_name, waza_name )
    @root_mons_name = mons_name
    @target_waza_name = waza_name
    @route_found_flag = false
    @valid_route = nil
    root = EggWazaDeriveRoute.new
    root.add( mons_name )
    @current_route.clear
    @current_route << root
  end

  def full_trace( mons_name, waza_name )
    self.init_trace( mons_name, waza_name )
    while( @route_found_flag == false && @current_route.size != 0 )
      self.trace_all_current_route
    end
    return @valid_route
  end

  def trace_all_current_route
    @next_route = Array.new
    @current_route.each do |route|
      self.trace( route )
    end
    @current_route = @next_route
    @next_route = Array.new
  end

  def trace( route )
    mons_name = route.last
    waza_checker = EggWazaChecker.new
    result = waza_checker.partial_analyze( mons_name, [@target_waza_name] )
    @checked_mons_list << mons_name # チェック済みリストに登録
    if result.is_valid( @target_waza_name ) then
      @route_found_flag = true
      @valid_route = route
      @valid_route.add( result.get( @target_waza_name ) )
    else
      father_list = waza_checker.get_father_list
      father_list = self.delete_checked_monsters_from_father_list( father_list ) # 再帰的に出現するチェック済みのルートを枝刈り
      father_list = self.delete_invalid_monsters_from_father_list( father_list ) # 覚える見込みのない父候補を除外

      if father_list != nil then
        self.add_next_route( route, father_list )
      end
    end
  end

  def delete_checked_monsters_from_father_list( father_list )
    new_father_list = Array.new
    if father_list != nil then
      father_list.each do |father|
        if @checked_mons_list.include?( father ) == false then
          new_father_list << father
        end
      end
    end
    return new_father_list
  end

  def delete_invalid_monsters_from_father_list( father_list )
    new_father_list = Array.new
    personal_accessor = PersonalAccessor.new
    if father_list != nil then
      father_list.each do |father|
        if personal_accessor.check_waza_learning( father, 100, @target_waza_name ) then
          new_father_list << father
        end
      end
    end
    return new_father_list
  end

  def add_next_route( route, father_list )
    father_list.each do |father|
      new_route = route.copy
      new_route.add( father )
      @next_route << new_route
    end
  end

end
