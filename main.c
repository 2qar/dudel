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

	const int cursor_size = 16;
	Uint32 colors[2] = {
		SDL_MapRGB(window_surface->format, 255, 255, 255),
		SDL_MapRGB(window_surface->format, 0, 0, 0),
	};
	int color = 0;
	SDL_Event e;
	int running = 1;
	int draw = 0;
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
						memset(window_surface->pixels, 0, window_surface->pitch * window_surface->h);
						break;
					case SDL_SCANCODE_Z:
						if (e.key.keysym.mod & KMOD_CTRL)
							memcpy(window_surface->pixels, backup, window_surface->pitch * window_surface->h);
						break;
					default:
						break;
				}
			} else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				draw = 1;
				memcpy(backup, window_surface->pixels, window_surface->pitch * window_surface->h);
			} else if (e.type == SDL_MOUSEBUTTONUP && e.button.button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				draw = 0;
			}
		}

		if (draw) {
			SDL_PumpEvents();
			int x, y;
			if (SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				SDL_Rect mouse = { x-cursor_size/2, y-cursor_size/2, cursor_size, cursor_size };
				/* TODO: have a canvas surface so i can draw a mouse cursor thing seperately */
				SDL_FillRect(window_surface, &mouse, colors[color]);
			}
		}
		SDL_Rect corner = { 640-32, 480-32, 32, 32 };
		SDL_FillRect(window_surface, &corner, colors[color]);

		SDL_UpdateWindowSurface(w);
	}

	free(backup);
	SDL_DestroyWindow(w);
	SDL_Quit();
	return 0;
}
