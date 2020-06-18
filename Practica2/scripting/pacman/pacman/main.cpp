#include <pacman_include.hpp>
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


struct Color
{
	float r;
	float g;
	float b;

	Color(float _r, float _g, float _b) : r(_r), g(_g), b(_b)
	{

	}
};
bool m_bColorInitialized = false;
int num_coins = 0;
const int platas_para_oro = 5;
const int bronces_para_plata = 100;

const float max_vida = 1.5f;
float vida = max_vida; 

lua_State* m_pLua;

int m_iCoinScore = 0, m_iPowerupScore = 0, m_iScoreToBronze = 0;
float m_fPowerupDuration = 0, m_fSpeedMultiplicator = 0, m_fTimeEaten;
Color m_oPowerupColor(0, 0, 0);
time_t  m_oLastTimeEaten, m_oTimeNow;
bool m_bHasBeenEaten;

Color ColorFromLua(int _iIndex)
{
	lua_getfield(m_pLua, _iIndex, "r");
	float fRed = lua_tonumber(m_pLua, -1) * 255;
	lua_getfield(m_pLua, _iIndex, "g");
	float fGreen = lua_tonumber(m_pLua, -1) * 255;
	lua_getfield(m_pLua, _iIndex, "b");
	float fBlue = lua_tonumber(m_pLua, -1) * 255;

	return Color(fRed, fGreen, fBlue);
}
Color ColorFromLivesLua(const char* _sName, float _fParameter)
{
	lua_getglobal(m_pLua, _sName);
	lua_pushnumber(m_pLua, _fParameter);
	lua_pcall(m_pLua, 1, 1, 0);
	lua_getfield(m_pLua, -1, "r");
	float fRed = lua_tonumber(m_pLua, -1) * 255;
	lua_getfield(m_pLua, -2, "g");
	float fGreen = lua_tonumber(m_pLua, -1) * 255;
	lua_getfield(m_pLua, -3, "b");
	float fBlue = lua_tonumber(m_pLua, -1) * 255;

	return Color(fRed, fGreen, fBlue);
}


void SetPacmanColor(Color _oColor)
{
	setPacmanColor((int)_oColor.r, (int)_oColor.g, (int)_oColor.b);
}

bool InitializeLua() 
{
	m_pLua = luaL_newstate(); /* crea el entorno de lua */
	luaL_openlibs(m_pLua); /* abre las librerias */
	int error = luaL_loadfile(m_pLua, "lua/pacman.lua"); /* carga el codigo en la pila */
	error |= lua_pcall(m_pLua, 0, 0, 0); /* ejecuta el codigo */
	if (error) {
		fprintf(stderr, "%s", lua_tostring(m_pLua, -1)); /* el mensaje de error esta en la cima de la pila */
		lua_pop(m_pLua, 1); /* quitar el mensaje de error de la pila */
		return false;
	}
	else
	{
		lua_getglobal(m_pLua, "coin_score");
		lua_getglobal( m_pLua,  "powerup_score");
		lua_getglobal(m_pLua, "score_to_bronze");
		lua_getglobal(m_pLua, "powerup_duration");
		lua_getglobal(m_pLua, "speed_multiplicator");
		lua_getglobal(m_pLua, "powerup_color");

		m_iCoinScore = lua_tointeger(m_pLua, 1);
		m_iPowerupScore = lua_tointeger(m_pLua, 2);
		m_iScoreToBronze = lua_tointeger(m_pLua, 3);
		m_fPowerupDuration = lua_tonumber(m_pLua, 4);
		m_fSpeedMultiplicator = lua_tonumber(m_pLua, 5);

		m_oPowerupColor = ColorFromLua(6);

		return true;
	}
}

void FinalizeLua()
{
	lua_close(m_pLua); /* cierra el entorno */
}


bool pacmanEatenCallback(int& score, bool& muerto)
{ // Pacman ha sido comido por un fantasma
	vida -= 0.5f;
	muerto = vida < 0.0f;
	if (!m_bHasBeenEaten)
	{
		m_bHasBeenEaten = true;
	}

	return true;
}

bool coinEatenCallback(int& score)
{ // Pacman se ha comido una moneda
	++num_coins;
	score = num_coins * m_iCoinScore;

	return true;
}

bool frameCallback(float time)
{ // Se llama periodicamente cada frame
	

	if (m_bHasBeenEaten)
	{
		m_fTimeEaten += time;
		if (m_fTimeEaten > 2.05f)
		{
			SetPacmanColor(ColorFromLivesLua("colorFromLives", vida));
			m_fTimeEaten = 0;
			m_bHasBeenEaten = false;
		}
	}

	return false;
}

bool ghostEatenCallback(int& score)
{ // Pacman se ha comido un fantasma
	return false;
}

bool powerUpEatenCallback(int& score)
{ // Pacman se ha comido un powerUp
	setPacmanSpeedMultiplier(m_fSpeedMultiplicator);
	SetPacmanColor(m_oPowerupColor);
	setPowerUpTime(m_fPowerupDuration);

	score += m_iPowerupScore;

	return true;
}

bool powerUpGone()
{ // El powerUp se ha acabado
	SetPacmanColor(ColorFromLivesLua("colorFromLives", vida));
	setPacmanSpeedMultiplier(1.0f);
	return true;
}

bool pacmanRestarted(int& score)
{
	score = 0;
	num_coins = 0;
	vida = max_vida;
	m_bColorInitialized = false;

	return true;
}

bool computeMedals(int& oro, int& plata, int& bronce, int score)
{
	bronce = score / m_iScoreToBronze;

	plata = bronce / bronces_para_plata;
	bronce = bronce % bronces_para_plata;
	
	oro = plata / platas_para_oro;
	plata = plata % platas_para_oro;

	return true;
}

bool getLives(float& vidas)
{
	vidas = vida;
	if (!m_bColorInitialized)
	{
		SetPacmanColor(ColorFromLivesLua("colorFromLives", vida));
		m_bColorInitialized = true;
	}
	//SetPacmanColor(ColorFromLivesLua("colorFromLives", vida));
	return true;
}

bool setImmuneCallback()
{
    return true;
}

bool removeImmuneCallback()
{
    return true;
}

bool InitGame()
{
	InitializeLua();
	m_bColorInitialized = false;
	return true;
}

bool EndGame()
{
	FinalizeLua();
    return true;
}