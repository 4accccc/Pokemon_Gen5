# ƒ^ƒ}ƒS‹Z‚ÌŒp³ƒ‹[ƒg
class EggWazaDeriveRoute

  def initialize
    @route = Array.new
  end

  def add( node )
    @route << node
  end

  def get
    return @route
  end

  def last
    return @route.last
  end

  def include?( node )
    return @route.include?( node )
  end

  def copy
    copy_route = EggWazaDeriveRoute.new
    @route.each do |node|
      copy_route.add( node )
    end
    return copy_route
  end

  def to_s
    string = ""
    @route.each do |node|
      string += "#{node} <-- "
    end
    string.chomp!( " <-- " ) # ÅŒã‚Ì <-- ‚ğíœ
    return string
  end

end 
