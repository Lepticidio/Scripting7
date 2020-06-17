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
score_to_bronze = 10
bronze_to_silver = 10
silver_to_gold = 10
powerup_duration = 20
speed_multiplicator = 2

--defino el color de 0 a 1 porque me es más intuitivo
powerup_color = Color.new(0.5, 0, 1)


function colorFromLives(_lives)
    if _lives > 1 then
        return Color.new(1, 1, 0)
    elseif _lives > 0.5 then
        return Color.new(1, 0.5, 0)
    elseif _lives > 0 then
        return Color.new(0, 1, 0)
    else
        return Color.new(0, 0, 1)
    end
end

function computeMedals(_score)

    bronze = math.floor(_score / score_to_bronze)

    silver = math.floor(bronze / bronze_to_silver)
    bronze = bronze - silver*bronze_to_silver

    gold = math.floor(silver / silver_to_gold)
    silver = silver - gold*silver_to_gold

    return gold, silver, bronze
end
function pacmanEaten(_lives)
    decreaseOneLife()
    dead = false
    if _lives - 0.5  < 0 then
        dead = true
    end
    return dead
end

function getWidth()
	return width
end

function frameUpdate(_time, _timeEaten, _lives)

	if _timeEaten > 2.05 then
		setPacmanColor(colorFromLives(_lives).r, colorFromLives(_lives).g, colorFromLives(_lives).b)
	end
    updateTimeEaten(_time)
	return false
end

function powerupEaten()
    setPacmanSpeedMultiplier(speed_multiplicator)
    setPacmanColor(powerup_color.r, powerup_color.g, powerup_color.b)
    setPowerUpTime(powerup_duration)
end

function initializePacman(_lives)
    resetPacman()
    setPacmanColor(colorFromLives(_lives).r, colorFromLives(_lives).g, colorFromLives(_lives).b)

end
function powerUpGone(_lives)
    setPacmanColor(colorFromLives(_lives).r, colorFromLives(_lives).g, colorFromLives(_lives).b)
    setPacmanSpeedMultiplier(1)
end
