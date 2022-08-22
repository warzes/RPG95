#pragma once

#define FLIP_PIXEL_BUFFER_Y 0
#define VIEWPORT_X 160
#define VIEWPORT_Y 120

//#define VIEWPORT_X 1280
//#define VIEWPORT_Y 1024

template<size_t W, size_t H>
class PixelBuffer
{
public:
	PixelBuffer(SDL_Renderer* renderer)
	{
		m_renderer = renderer;
		m_textureSDL = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STATIC, W, H);
	}

	~PixelBuffer()
	{
		SDL_DestroyTexture(m_textureSDL);
	}

	void Clear()
	{
		memset(m_pixels, 0, W * H * sizeof(uint32_t));
	}

	void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
	{
		// TODO: alpha
		if (x < 0 || y < 0 || x >= W || y >= H) return;
#if FLIP_PIXEL_BUFFER_Y
		y = H - y - 1;
#endif
		uint8_t* p = ((uint8_t*)m_pixels) + (y * W + x) * 4 + 1;

		*p = blue;
		++p;
		*p = green;
		++p;
		*p = red;
	}

	void Draw()
	{
		SDL_UpdateTexture(m_textureSDL, nullptr, m_pixels, W * sizeof(uint32_t));
		SDL_RenderCopy(m_renderer, m_textureSDL, nullptr, nullptr);
	}

private:
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_textureSDL = nullptr;
	uint32_t m_pixels[W * H] = { 200 };
};

extern PixelBuffer<VIEWPORT_X, VIEWPORT_Y>* gPixelBuffer;