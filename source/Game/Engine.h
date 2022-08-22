#pragma once



#include "Core.h"
#include "PixelBuffer.h"

#define RESOLUTION_X 1024
#define RESOLUTION_Y 768

enum class GameKey
{
	W,
	S,
	A,
	D,
	M,
	Q,
	E,
};
constexpr size_t MaxGameKey = 7;

class Input
{
public:
	Input()
	{
		m_keyMapping[SDLK_w] = GameKey::W;
		m_keyMapping[SDLK_a] = GameKey::A;
		m_keyMapping[SDLK_s] = GameKey::S;
		m_keyMapping[SDLK_d] = GameKey::D;
		m_keyMapping[SDLK_m] = GameKey::M;
		m_keyMapping[SDLK_q] = GameKey::Q;
		m_keyMapping[SDLK_e] = GameKey::E;
	}
	void SetKey(SDL_Event sdlEvent, bool state)
	{
		auto it = m_keyMapping.find(sdlEvent.key.keysym.sym);
		if (it != m_keyMapping.end())
		{
			const size_t id = static_cast<size_t>(it->second);

			m_keyDown[id] = state;
			if (!state) m_keyPress[id] = false;
		}
	}

	bool IsKeyDown(GameKey key)
	{
		return m_keyDown[static_cast<size_t>(key)];
	}
	bool IsKeyPress(GameKey key)
	{
		if (!IsKeyDown(key) || m_keyPress[static_cast<size_t>(key)]) return false;
		m_keyPress[static_cast<size_t>(key)] = true;
		return true;
	}
private:
	std::map<SDL_Keycode, GameKey> m_keyMapping;
	bool m_keyDown[MaxGameKey] = { false };
	bool m_keyPress[MaxGameKey] = { false };
};

extern Input gInput;