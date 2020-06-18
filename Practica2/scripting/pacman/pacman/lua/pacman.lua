Color = {}
Color["r"] = 0
Color["g"] = 0
Color["b"] = 0

function Color.new(_r, _g, _b)
    local color = {}
    color.r = _r
    color.g = _g
    color.b = _b
    Color.__index = Color
    setmetatable(color, Color)
    return color
end

width = 300
coin_score = 25
powerup_score = 2500
score_to_bronze = 2
powerup_duration = 20
speed_multiplicator = 2
--defino el color de 0 a 1 porque me es más intuitivo
powerup_color = Color.new(0.5, 0, 1)


function colorFromLives(_lives)
    if _lives > 1 then
        return Color.new(1, 0, 0)
    elseif _lives > 0.5 then
        return Color.new(1, 0.5, 0)
    elseif _lives > 0 then
        return Color.new(0, 1, 0)
    else
        return Color.new(0, 0, 1)
    end
end

function getWidth()
	return width
end
