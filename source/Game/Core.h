#pragma once

inline void PrintLog(const std::string& text)
{
#if defined(__EMSCRIPTEN__)
	puts(text.c_str());
#else
	SDL_Log(text.c_str());
#endif
}

inline void ErrorLog(const std::string& text)
{
	PrintLog("ERROR: " + text);
}