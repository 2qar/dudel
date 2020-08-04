#include <stdlib.h>
#include <SDL2/SDL.h>

int main() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("error initalizing video: %s", SDL_GetError());
		return 1;
	}

	SDL_Window *w = SDL_CreateWindow("dudel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
	if (w == NULL) {
		SDL_Log("error creating window: %s", SDL_GetError());
		return 1;
	}

	SDL_Surface *window_surface = SDL_GetWindowSurface(w);
	SDL_Log("buf size is %d", window_surface->pitch * window_surface->h);
	/* TODO: more undos, but find a smart way to do it.
	 *       allocating more undo buffers will take up a lotta memory (100 undos = 128mb ram) */
	void *backup = malloc(window_surface->pitch * window_surface->h);

	SDL_Surface *canvas = SDL_CreateRGBSurfaceWithFormat(0,
			window_surface->w, window_surface->h,
			window_surface->format->BitsPerPixel,
			window_surface->format->format);

	Uint32 colors[2] = {
		SDL_MapRGB(window_surface->format, 255, 255, 255),
		SDL_MapRGB(window_surface->format, 0, 0, 0),
	};
	int color = 0;
	SDL_Event e;
	int running = 1;
	int draw = 0;
	int mouse_x = 0, mouse_y = 0;
	int cursor_size = 16;
	const int cursor_thickness = 1;
	while (running) {
		window_surface = SDL_GetWindowSurface(w);
		if (SDL_WaitEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = 0;
			} else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_Q:
						running = 0;
						break;
					case SDL_SCANCODE_X:
						color = !color;
						break;
					case SDL_SCANCODE_R:
						memset(canvas->pixels, 0, canvas->pitch * canvas->h);
						break;
					case SDL_SCANCODE_Z:
						if (e.key.keysym.mod & KMOD_CTRL)
							memcpy(canvas->pixels, backup, canvas->pitch * canvas->h);
						break;
					default:
						break;
				}
			} else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				draw = 1;
				memcpy(backup, canvas->pixels, canvas->pitch * canvas->h);
			} else if (e.type == SDL_MOUSEBUTTONUP && e.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				draw = 0;
			} else if (e.type == SDL_MOUSEMOTION) {
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
			} else if (e.type == SDL_MOUSEWHEEL) {
				if (e.wheel.y > 0)
					cursor_size += 1;
				else
					cursor_size -= 1;
			}
		}

		if (draw) {
			SDL_PumpEvents();
			int x, y;
			if (SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				SDL_Rect mouse = { x-cursor_size/2, y-cursor_size/2, cursor_size, cursor_size };
				SDL_FillRect(canvas, &mouse, colors[color]);
			}
		}

		SDL_Rect window_size = { 0, 0, window_surface->w, window_surface->h };
		SDL_BlitSurface(canvas, &window_size, window_surface, &window_size);

		SDL_Rect corner = { window_surface->w-32, window_surface->h-32, 32, 32 };
		SDL_FillRect(window_surface, &corner, colors[color]);

		/* draw the cursor */
		SDL_Rect cursor = { mouse_x, mouse_y, 0, 0 };
		cursor.x -= cursor_size / 2;
		cursor.y -= cursor_size / 2;
		cursor.w = cursor_size;
		cursor.h = cursor_thickness;
		SDL_FillRect(window_surface, &cursor, colors[color]);
		cursor.y += cursor_size;
		SDL_FillRect(window_surface, &cursor, colors[color]);
		cursor.y -= cursor_size;
		cursor.h = cursor.w;
		cursor.w = cursor_thickness;
		SDL_FillRect(window_surface, &cursor, colors[color]);
		cursor.x += cursor_size;
		SDL_FillRect(window_surface, &cursor, colors[color]);

		SDL_UpdateWindowSurface(w);
	}

	free(backup);
	SDL_FreeSurface(canvas);
	SDL_DestroyWindow(w);
	SDL_Quit();
	return 0;
}
