#include <pacman_include.hpp>
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"



bool m_bInitialized = false, m_bHasBeenEaten;
int num_coins = 0;
float m_fTimeEaten, m_fLives = 1.5f;

lua_State* m_pLua;

int m_iCoinScore = 0, m_iPowerupScore = 0;
time_t  m_oLastTimeEaten, m_oTimeNow;


int SetPacmanColor(lua_State* _pLua)
{
	float fRed = lua_tonumber(_pLua, 1) * 255;
	float fGreen = lua_tonumber(_pLua, 2) * 255;
 	float fBlue = lua_tonumber(_pLua, 3) * 255;
	setPacmanColor((int)fRed, (int)fGreen, (int)fBlue);
	return 0;
}

int SetPacmanSpeedMultiplier(lua_State* _pLua)
{
	float fSpeedMultiplicator = lua_tonumber(_pLua, 1);
	setPacmanSpeedMultiplier(fSpeedMultiplicator);
	return 0;
}

int SetPowerupTime(lua_State* _pLua)
{
	float fPowerupDuration = lua_tonumber(_pLua, 1);
	setPowerUpTime(fPowerupDuration);
	return 0;
}

int UpdateTimeEaten(lua_State* _pLua)
{
	float fTime = lua_tonumber(_pLua, 1);
	if (m_bHasBeenEaten)
	{
		if (m_fTimeEaten > 2.05f)
		{
			m_fTimeEaten = 0;
			m_bHasBeenEaten = false;
		}
		m_fTimeEaten += fTime;
	}
	return 0;
}

int ResetPacman(lua_State* _pLua)
{
	num_coins = 0;
	m_fLives = 1.5f;
	return 0;
}

int DecreaseOneLife(lua_State* _pLua)
{
	m_fLives -= 0.5f;
	if (!m_bHasBeenEaten)
	{
		m_bHasBeenEaten = true;
	}
	return 0;
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

		lua_settop(m_pLua, 0);
		lua_getglobal(m_pLua, "coin_score");
		lua_getglobal(m_pLua, "powerup_score");

		lua_pushcfunction(m_pLua, SetPacmanColor);
		lua_setglobal(m_pLua, "setPacmanColor");
		lua_pushcfunction(m_pLua, SetPacmanSpeedMultiplier);
		lua_setglobal(m_pLua, "setPacmanSpeedMultiplier");
		lua_pushcfunction(m_pLua, SetPowerupTime);
		lua_setglobal(m_pLua, "setPowerUpTime");
		lua_pushcfunction(m_pLua, UpdateTimeEaten);
		lua_setglobal(m_pLua, "updateTimeEaten");
		lua_pushcfunction(m_pLua, DecreaseOneLife);
		lua_setglobal(m_pLua, "decreaseOneLife");
		lua_pushcfunction(m_pLua, ResetPacman);
		lua_setglobal(m_pLua, "resetPacman");


		m_iCoinScore = lua_tointeger(m_pLua, 1);
		m_iPowerupScore = lua_tointeger(m_pLua, 2);

		return true;
	}
}

void FinalizeLua()
{
	lua_close(m_pLua); /* cierra el entorno */
}


bool pacmanEatenCallback(int& score, bool& muerto)
{ // Pacman ha sido comido por un fantasma


	lua_getglobal(m_pLua, "pacmanEaten");
	lua_pushnumber(m_pLua, m_fLives);
	lua_pcall(m_pLua, 1, 1, 0);
	muerto = lua_toboolean(m_pLua, -1);
	return true;
}

bool coinEatenCallback(int& score)
{ // Pacman se ha comido una moneda
	++num_coins;
	score = num_coins * m_iCoinScore;

	return true;
}

bool frameCallback(float time)
{ 
	// Se llama periodicamente cada frame	

	InitializeLua();
	if (!m_bInitialized)
	{
		lua_getglobal(m_pLua, "initializePacman");
		lua_pushnumber(m_pLua, m_fLives);
		lua_pcall(m_pLua, 1, 0, 0);
		m_bInitialized = true;
	}
	lua_getglobal(m_pLua, "frameUpdate");
	lua_pushnumber(m_pLua, time);
	lua_pushnumber(m_pLua, m_fTimeEaten);
	lua_pushnumber(m_pLua, m_fLives);
	lua_pcall(m_pLua, 3, 1, 0);



	return false;
}

bool ghostEatenCallback(int& score)
{ // Pacman se ha comido un fantasma
	return false;
}

bool powerUpEatenCallback(int& score)
{ // Pacman se ha comido un powerUp
	lua_getglobal(m_pLua, "powerupEaten");
	lua_pcall(m_pLua, 0, 0, 0);
	score += m_iPowerupScore;
	return true;
}

bool powerUpGone()
{ // El powerUp se ha acabado
	lua_getglobal(m_pLua, "powerUpGone");
	lua_pushnumber(m_pLua, m_fLives);
	lua_pcall(m_pLua, 1, 0, 0);
	return true;
}

bool pacmanRestarted(int& score)
{
	score = 0; 
	m_bInitialized = false;
	return true;
}

bool computeMedals(int& oro, int& plata, int& bronce, int score)
{
	lua_getglobal(m_pLua, "computeMedals");
	lua_pushnumber(m_pLua, score);
	lua_pcall(m_pLua, 1, 3, 0);
	oro = lua_tointeger(m_pLua, -3);
	plata = lua_tointeger(m_pLua, -2);
	bronce = lua_tointeger(m_pLua, -1);

	return true;
}

bool getLives(float& vidas)
{
	lua_getglobal(m_pLua, "lives");
	vidas = lua_tonumber(m_pLua, -1);
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
	return true;
}

bool EndGame()
{
	FinalizeLua();
    return true;
}