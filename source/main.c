/* ISMET BARAN SURUCU -- github.com/baransrc */
#include "tetris_util.h"
#include <stdlib.h>  
#include <stdio.h>
#include <stdbool.h>
#include "../include/SDL.h"
#include "../include/SDL_ttf.h"
#include <time.h>

#define FRAME_PER_SECOND_CAP 60
#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 768	
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 22
#define BOARD_HEIGHT_RENDERED 20
#define TETROMINO_SIZE 32
#define BOARD_OFFSET_X 32
#define BOARD_OFFSET_Y 32
#define BOARD_SIZE BOARD_HEIGHT*BOARD_WIDTH
#define MAX_TETROMINO_WIDTH 5
#define MAX_TETROMINO_HEIGHT 5
#define MAX_TETROMINO_ARRAY_SIZE MAX_TETROMINO_WIDTH*MAX_TETROMINO_HEIGHT
#define TETROMINO_TYPE_COUNT 7
#define TETROMINO_ROTATION_COUNT 4
#define EMPTY_CELL_TYPE 255
#define TETROMINO_PIVOT_X 2
#define TETROMINO_PIVOT_Y 2
#define TEXT_BUFFER_SIZE 1024

static const char* FILE_PATH_SPLASH_SCREEN = "..\\assets\\images\\baran_logo.bmp";
static const char* FILE_PATH_MAIN_FONT = "..\\assets\\fonts\\Montserrat-Semibold.ttf";
static const float_t DURATION_LINE_ANIMATION = 0.2f;

enum Text_Alignment
{
	TEXT_ALIGNMENT_LEFT,
	TEXT_ALIGNMENT_CENTER,
	TEXT_ALIGNMENT_RIGHT,
};

enum Text_Render_Mode
{
	TEXT_RENDER_MODE_SOLID,
	TEXT_RENDER_MODE_SHADED,
	TEXT_RENDER_MODE_BLENDED,
};

enum Tetromino_Type
{
	TETROMINO_TYPE_I, 
	TETROMINO_TYPE_O, 
	TETROMINO_TYPE_T, 
	TETROMINO_TYPE_J, 
	TETROMINO_TYPE_L, 
	TETROMINO_TYPE_S, 
	TETROMINO_TYPE_Z
};

enum Game_Phase
{	
	GAME_PHASE_PLAYING,
	GAME_PHASE_GAMEOVER,
};

typedef struct Vector2
{
	int16_t x;
	int16_t y;
} Vector2;

typedef struct Extents
{
	int16_t min_x;
	int16_t min_y;
	int16_t max_x;
	int16_t max_y;
} Extents;

typedef struct Tetromino
{
	Vector2 pivot_position;
	uint8_t rotation; 
	enum Tetromino_Type type;
} Tetromino;

typedef struct Input_State
{
	bool pressed_left;
	bool pressed_right;
	bool pressed_up;
	bool pressed_down; 
	bool pressed_space;
} Input_State;

typedef struct Game_State
{
	uint8_t board[BOARD_SIZE];
	uint8_t previous_tetromino_rotation;
	double delta_time;
	float_t fall_clock;	 
	enum Game_Phase game_phase;
	bool should_spawn_tetromino;
	Vector2 current_destination;
	Vector2 previous_tetromino_position;
	Tetromino current_tetromino;
	uint32_t line_count;
	uint32_t score;
	uint8_t current_level;
	float tetromino_lines[BOARD_HEIGHT_RENDERED];
} Game_State;

typedef struct Text
{
	char buffer[TEXT_BUFFER_SIZE];
	enum Text_Alignment alignment;
	Vector2 position;
} Text;

typedef struct Text_State
{
	Text score_text;
	Text line_text;
	Text level_text;
} Text_State;

// SDL --------------------------
void update_window_name(SDL_Window*, int, double);
bool initialize_window(SDL_Window**,  SDL_Surface**, int, int);
bool initialize_renderer(SDL_Window*, SDL_Renderer**);
bool load_bmp_image(SDL_Surface**, char*);
// ------------------------------

// Utils ------------------------
inline SDL_Color color_to_sdl_color(Color);
void draw_text(SDL_Renderer*, TTF_Font*, char*, Vector2, enum Text_Alignment, enum Text_Render_Mode, Color);
void draw_filled_rectangle(SDL_Renderer*, int, int, int, int, Color);
int random_range(int, int);
inline void set_2d_array_element(uint8_t*, int, int, int, uint8_t);
inline uint8_t get_2d_array_element(uint8_t*, int, int, int);
inline int16_t get_x_extent_relative_to_board(int16_t, int16_t);
inline int16_t get_y_extent_relative_to_board(int16_t, int16_t);
Extents find_extents_of_tetromino(Tetromino);
// ------------------------------

// Gameplay ---------------------
bool is_possible_movement(Game_State*, bool);
void clamp_movement(Game_State*);
void put_tetromino_to_board(Game_State*, bool);
void delete_tetromino_from_board(Game_State*, enum Tetromino_Type, uint16_t, uint16_t, uint8_t);
void move_tetromino_for_rotation(Game_State*);
inline void level_up(Game_State*);
inline void add_score(Game_State*, uint8_t);
inline double get_current_fall_time(Game_State*);
bool recycle_current_tetromino(Game_State*);
inline bool will_fall_this_turn(Game_State*);
bool tetromino_fall(Game_State*);
void determine_current_destination(Game_State*);
void check_game_over(Game_State*);
void destroy_lines(Game_State*);
void update_line_data(Game_State*);
void update_game_text(Game_State*, Text_State*);
void update_game_playing_phase(Game_State*, Input_State*);
void update_game_gameover_phase(Game_State*, Input_State*);
void update_game(Game_State*, Input_State*);
void initialize_game_state(Game_State*);
void initialize_game(Game_State*, Input_State*, Text_State*);
void reset_input_state(Input_State*);
void parse_input_state_playing_phase(Game_State*, Input_State*);
// ------------------------------

// Rendering --------------------
void render_game_text_playing_phase(Game_State*, Text_State*, SDL_Renderer*, TTF_Font*, TTF_Font*);
void render_game_text_gameover_phase(Game_State*, Text_State*, SDL_Renderer*, TTF_Font*, TTF_Font*);
void render_game_text(Game_State*, Text_State*, SDL_Renderer*, TTF_Font*, TTF_Font*);
void draw_tetromino_unit(SDL_Renderer*, int, int, enum Tetromino_Type);
void draw_empty_cell(SDL_Renderer*, int, int);
void draw_current_destination(Game_State*, SDL_Renderer*);
void draw_tetrominoes(Game_State*, SDL_Renderer*);
void draw_board_cells(Game_State*, SDL_Renderer*);
void draw_lines(Game_State* game_state, SDL_Renderer* renderer);
void render_game_playing_phase(Game_State*, SDL_Renderer*);
void render_game_gameover_phase(Game_State*, SDL_Renderer*);
void render_game(Game_State*, SDL_Renderer*);
// ------------------------------


int main( int argc, char* args[] )
{
	// Set the seed for random number generator:
	srand(time(NULL)); 

	// The window that will be rendered to:
	SDL_Window* window = NULL;
	// The surface contained by the window:
	SDL_Surface* screen_surface = NULL;
	// Initialize Window:
	bool window_initialization_success = initialize_window(&window, &screen_surface, SCREEN_WIDTH, SCREEN_HEIGHT);

	if (window_initialization_success)
	{
		// Initialize SDL_TTF:
		if (TTF_Init() < 0)
		{
			printf("TTF Could not be loaded!\n");
			system("pause");
			return 1;
		}

		// Splash screen image:
		SDL_Surface* splash_screen_surface = NULL;
		// Load splash screen image:
		bool splash_screen_loaded = load_bmp_image(&splash_screen_surface, FILE_PATH_SPLASH_SCREEN);

		if (splash_screen_loaded)
		{
			// Blit splash screen image to screen surface:
			SDL_BlitSurface(splash_screen_surface, NULL, screen_surface, NULL);
			// Update the surface:
			SDL_UpdateWindowSurface(window);
			// Wait for a second:
			SDL_Delay(1000);

			// Deallocate Splash Screen Surface:
			SDL_FreeSurface(splash_screen_surface);
			splash_screen_surface = NULL;		
		}

		// Renderer that window uses:
		SDL_Renderer* renderer = NULL;
		// Initialize Renderer:
		bool renderer_initialization_success = initialize_renderer(window, &renderer); 

		// Game Fonts:
		TTF_Font* font_24pt = TTF_OpenFont(FILE_PATH_MAIN_FONT, 24);
		TTF_Font* font_16pt = TTF_OpenFont(FILE_PATH_MAIN_FONT, 16);

		if (renderer_initialization_success)
		{	
			bool user_quit = false;
			
			// SDL_Event holds event data which will be parsed by Input_State:
			SDL_Event event_container;

			// Delta Time related:
			uint8_t refresh_frame_rate = 0;
			int64_t time_now = SDL_GetTicks();
			int64_t time_last = SDL_GetTicks();
			double delta_time = 0;
			double delta_time_ms = time_now - time_last;

			// Game related
			Game_State game_state;
			Input_State input_state;
			Text_State text_state;

			// Initialize game_state, input_state and text_state:
			initialize_game(&game_state, &input_state, &text_state);

			while (!user_quit)
			{
				while (SDL_PollEvent(&event_container) != 0)
				{
					// On User Requests Quit:
					if (event_container.type == SDL_QUIT)
					{
						user_quit = true;
					}
					else if (event_container.type == SDL_KEYDOWN)
					{
						switch( event_container.key.keysym.sym )
                        {
                            case SDLK_UP:
							input_state.pressed_up = true;
                            break;

                            case SDLK_DOWN:
							input_state.pressed_down = true;
                            break;

                            case SDLK_LEFT:
                            input_state.pressed_left = true;
							break;

                            case SDLK_RIGHT:
							input_state.pressed_right = true;
                            break;

							case SDLK_SPACE:
							input_state.pressed_space = true;
							break;

                            default:
                            break;
                        }
					}
				}
				
				// Time of this frame:
				time_now = SDL_GetTicks();
				
				// Get delta time in milliseconds:
				delta_time_ms = time_now - time_last;

				// If this frame took less than expected, do delay:
				if (delta_time_ms < (1000.0/FRAME_PER_SECOND_CAP))
				{	
					SDL_Delay((1000.0/FRAME_PER_SECOND_CAP) - delta_time_ms);
					
					time_now = SDL_GetTicks();
					
					delta_time_ms = time_now - time_last;
				}

				// // Calculate the time passed last frame:
				delta_time = delta_time_ms * 0.001;

				game_state.delta_time = delta_time;

				if (refresh_frame_rate == 0)
				{
					update_window_name(window, ((int)(1.0/delta_time)), delta_time*1000);
				}

				// Clear screen to black:
				SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
				SDL_RenderClear(renderer);
				
				// Update game logic:
				update_game(&game_state, &input_state);
				// Update text fields such as score, lines and level:
				update_game_text(&game_state, &text_state);
				
				// Render game according to it's phase:
				render_game(&game_state, renderer);
				// Render any text that needs to be rendered on screen:
				render_game_text(&game_state, &text_state, renderer, font_24pt, font_16pt);
				
				// Set all input flags to false:
				reset_input_state(&input_state);

				// Update Screen:
				SDL_RenderPresent(renderer);

				// Time of last frame:
				time_last = time_now;

				// Refresh framerate ~ every second:
				refresh_frame_rate = (refresh_frame_rate + 1) % FRAME_PER_SECOND_CAP; 
			}

			// Deallocate fonts:
			TTF_CloseFont(font_24pt);
			font_24pt = NULL;
			TTF_CloseFont(font_16pt);
			font_16pt = NULL;

			// Deallocate renderer:
			SDL_DestroyRenderer(renderer);
			renderer = NULL;
		}

		// Deallocate Main Screen Surface:
		SDL_FreeSurface(screen_surface);
		screen_surface = NULL;

		// Deallocate memory taken by window:
		SDL_DestroyWindow(window);
		window = NULL;
	}

	// Terminate SDL:
	SDL_Quit();

	return 0;
}

void update_window_name(SDL_Window* window, int fps, double ms)
{
	char window_name[64];
	
	// Write title with fps to window_name buffer:
	snprintf(window_name, sizeof(window_name), "TETRIS - FPS: %i (%.2fms)", fps, ms);
	// Set window name to the window_name:
	SDL_SetWindowTitle(window, window_name);
}

bool initialize_window(SDL_Window** window,  SDL_Surface** screen_surface, int screen_width, int screen_height)
{	
	bool success_flag = false;

	// Initialize SDL System
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

		system("pause");

		return success_flag;
	}

	// Create Window with SDL Context
	*window = SDL_CreateWindow("TETRIS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);

	if (*window == NULL)
	{
		printf("SDL Window could not be created! SDL_Error: %s\n", SDL_GetError());
		
		system("pause");
		
		return success_flag;
	}

	*screen_surface = SDL_GetWindowSurface(*window);
	success_flag = true;

	return success_flag;
}

bool initialize_renderer(SDL_Window* window, SDL_Renderer** renderer)
{
	bool success_flag = false;

	// Creating the renderer:
	*renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// Throw error if renderer could not be created:
	if (*renderer == NULL)
	{
		printf("SDL Renderer could not be created! SDL Error: %s\n", SDL_GetError());

		system("pause");
		
		return success_flag;
	}

	success_flag = true;

	return success_flag;
}	

bool load_bmp_image(SDL_Surface** screen_surface, char* filePath)
{
	bool success_flag = false;

	// Load image to surface:
	*screen_surface = SDL_LoadBMP(filePath);

	if (*screen_surface == NULL)
	{
		printf("Unable to load image: %s! SDL Error: %s\n", filePath, SDL_GetError());

		system("pause");
		
		return success_flag;
	}

	success_flag = true;

	return success_flag;
}

inline SDL_Color color_to_sdl_color(Color color)
{
	return (SDL_Color){color.r, color.g, color.b, color.a};
}

void draw_text(SDL_Renderer *renderer, TTF_Font* font, char* text, Vector2 text_position, enum Text_Alignment text_alignment, enum Text_Render_Mode render_mode, Color text_color)
{
	// Create surface :
	SDL_Surface* text_surface = NULL;
	switch (render_mode)
	{
	case TEXT_RENDER_MODE_SOLID:
		text_surface = TTF_RenderText_Solid(font, text, color_to_sdl_color(text_color));
		break;
	case TEXT_RENDER_MODE_SHADED:
		text_surface = TTF_RenderText_Shaded(font, text, color_to_sdl_color(text_color), color_to_sdl_color(TRANSPARENT_COLOR));
		break;
	case TEXT_RENDER_MODE_BLENDED:
		text_surface = TTF_RenderText_Blended(font, text, color_to_sdl_color(text_color));
		break;
	default:
		break;
	}

	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

	// Create the rect that texture will be rendered on:
	SDL_Rect text_rect = {.y = text_position.y, .w = text_surface->w, .h = text_surface->h};

	switch (text_alignment)
	{
	case TEXT_ALIGNMENT_LEFT:
		text_rect.x = text_position.x;
		break;
	
	case TEXT_ALIGNMENT_CENTER:
		text_rect.x = text_position.x - (text_surface->w / 2);
		break;
	
	case TEXT_ALIGNMENT_RIGHT:
		text_rect.x = text_position.x - text_surface->w;
		break;
	}

	// Copy texture created with text_texture and text_rect to the renderer:
	SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
	
	// Deallocate text_surface:
	SDL_FreeSurface(text_surface);
	text_surface = NULL;

	// Deallocate text_texture:
	SDL_DestroyTexture(text_texture);
	text_texture = NULL;
}

void draw_filled_rectangle(SDL_Renderer* renderer, int x_position, int y_position, int width, int height, Color color)
{
	SDL_Rect rectangle = 
	{
		.x = x_position,
		.y = y_position,
		.w = width,
		.h = height
	};

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rectangle);
}

int random_range(int min_n, int max_n)
{
    return rand() % (max_n - min_n + 1) + min_n;
}

inline void set_2d_array_element(uint8_t* array, int height, int row, int column, uint8_t value)
{
	array[(height * column) + row] = value;
}

inline uint8_t get_2d_array_element(uint8_t* array, int height, int row, int column)
{
	return array[(height * column) + row];
}

inline int16_t get_x_extent_relative_to_board(int16_t x_position, int16_t x_extent)
{
	return x_position + x_extent;
}

inline int16_t get_y_extent_relative_to_board(int16_t y_position, int16_t y_extent)
{
	return y_position - y_extent;
}

Extents find_extents_of_tetromino(Tetromino tetromino)
{
	// Finds extents of tetromino relative to matrix pivot position of tetromino (not board).

	Extents extents = {
		.max_x = 0, 
		.min_x = 0, 
		.max_y = 0, 
		.min_y = 0
	};

	for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
	{
		for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
		{
			uint8_t cell_value = TETROMINOES[tetromino.type][tetromino.rotation][j][i];

			if (cell_value == 0)
			{
				continue;
			}

			int16_t offset_x = i - TETROMINO_PIVOT_X;
			int16_t offset_y = j - TETROMINO_PIVOT_Y;
			
			extents.max_x = max(offset_x, extents.max_x);
			extents.min_x = min(offset_x, extents.min_x);
			extents.max_y = max(offset_y, extents.max_y);
			extents.min_y = min(offset_y, extents.min_y);
		}
	}
}

bool is_possible_movement(Game_State* game_state, bool force_update)
{
	Tetromino* tetromino = &(game_state->current_tetromino);
	Vector2 center = tetromino->pivot_position;
	uint8_t current_rotation = tetromino->rotation;
	enum Tetromino_Type type = tetromino->type;
	int final_x_offset = 0;

	if (center.x != game_state->previous_tetromino_position.x || 
	    center.y != game_state->previous_tetromino_position.y ||
		current_rotation != game_state->previous_tetromino_rotation || 
		force_update)
	{
		// Check if this will be a valid move:	
		for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
		{
			for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
			{
				uint8_t cell_value = TETROMINOES[type][current_rotation][j][i];

				if (cell_value == 0)
				{
					continue;
				}

				int offset_x = i - TETROMINO_PIVOT_X;
				int offset_y = j - TETROMINO_PIVOT_Y;
				int board_x = center.x + offset_x;
				int board_y = center.y - offset_y;

				// Is this cell overflowing from right:
				if (board_x >= BOARD_WIDTH )
				{	
					return false;
				}

				// Is this cell overflowing from left:
				if (board_x < 0)
				{
					return false;
				}

				// Is this cell overflowing from top:
				if (board_y >= BOARD_HEIGHT)
				{	
					return false;
				}

				// Is this cell overflowing from bottom:
				if (board_y < 0)
				{
					return false;
				}

				// Is this cell already occupied by another tetromino:
				if (get_2d_array_element(game_state->board, BOARD_WIDTH, board_x, board_y) != EMPTY_CELL_TYPE)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void clamp_movement(Game_State* game_state)
{
	if (!is_possible_movement(game_state, game_state->should_spawn_tetromino))
	{
		game_state->current_tetromino.pivot_position = game_state->previous_tetromino_position;
		game_state->current_tetromino.rotation = game_state->previous_tetromino_rotation;
	} 
}

void put_tetromino_to_board(Game_State* game_state, bool force_update)
{
	Tetromino* tetromino = &(game_state->current_tetromino);
	Vector2 position = tetromino->pivot_position;
	uint8_t rotation = tetromino->rotation;
	enum Tetromino_Type type = tetromino->type; 

	printf("--- Putting Tetromino (Pivot: %i,%i) ---\n", position.x, position.y);

	for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
	{
		for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
		{
			uint8_t cell_value = TETROMINOES[type][rotation][j][i];

			if (cell_value == 0)
			{
				continue;
			}

			int offset_x = i - TETROMINO_PIVOT_X;
			int offset_y = j - TETROMINO_PIVOT_Y;
			int board_x = position.x + offset_x;
			int board_y = position.y - offset_y;

			set_2d_array_element(game_state->board, BOARD_WIDTH, board_x, board_y, type);

			printf("Added Type: %i -- At: %i, %i -- Rotation: %i\n", get_2d_array_element(game_state->board, BOARD_WIDTH, board_x, board_y), board_x, board_y, rotation);
		}
	}

}

void delete_tetromino_from_board(Game_State* game_state, enum Tetromino_Type type, uint16_t x, uint16_t y, uint8_t rotation)
{
	for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
	{
		for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
		{
			uint8_t cell_value = TETROMINOES[type][rotation][j][i];

			if (cell_value == 0)
			{
				continue;
			}

			int offset_x = i - TETROMINO_PIVOT_X;
			int offset_y = j - TETROMINO_PIVOT_Y;
			int board_x = x + offset_x;
			int board_y = y - offset_y;

			set_2d_array_element(game_state->board, BOARD_WIDTH, board_x, board_y, EMPTY_CELL_TYPE);
		}
	}	 
}

void move_tetromino_for_rotation(Game_State* game_state)
{
	Tetromino* tetromino = &(game_state->current_tetromino);
	uint8_t previous_rotation = game_state->previous_tetromino_rotation;

	if (previous_rotation == tetromino->rotation)
	{
		return;
	}

	if (is_possible_movement(game_state, false))
	{
		return;
	}

	uint16_t position_x = tetromino->pivot_position.x;
	uint16_t position_y = tetromino->pivot_position.y;
	uint8_t rotation = tetromino->rotation;
	enum Tetromino_Type type = tetromino->type;
	int higher_max_x = 0;
	int lower_min_x = 0;

	for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
	{
		for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
		{
			uint8_t cell_value = TETROMINOES[type][rotation][j][i];

			if (cell_value == 0)
			{
				continue;
			}

			int offset_x = i - TETROMINO_PIVOT_X;
			int offset_y = j - TETROMINO_PIVOT_Y;
			int board_x = position_x + offset_x;
			int board_y = position_y - offset_y;

			int higher_difference_x = board_x - (BOARD_WIDTH - 1);
			int lower_difference_x = board_x;

			higher_max_x = max(higher_difference_x, higher_max_x);
			lower_min_x = min(lower_difference_x, lower_min_x);
		}
	}

	int final_offset_x = (higher_max_x) > 0 ? higher_max_x : lower_min_x;

	tetromino->pivot_position.x -= final_offset_x;
}

inline void level_up(Game_State* game_state)
{
	if ( game_state->current_level < (LEVEL_COUNT - 1) && 
		 game_state->line_count >= ((game_state->current_level + 1) * 10))
	{
		game_state->current_level++;
		printf("--- LEVEL: %i ---\n", game_state->current_level);
	}
}

inline void add_score(Game_State* game_state, uint8_t lines_this_frame)
{
	switch (lines_this_frame)
    {
		case 1:
			game_state->score += (40 * (game_state->current_level + 1));
		case 2:
			game_state->score += (100 * (game_state->current_level + 1));
		case 3:
			game_state->score += (300 * (game_state->current_level + 1));
		case 4:
			game_state->score += (1200 * (game_state->current_level + 1));
    }

	printf("--- SCORE: %i ---\n", game_state->score);
}

inline double get_current_fall_time(Game_State* game_state)
{
	return FALL_TIME_IN_SECS[game_state->current_level];
}

bool recycle_current_tetromino(Game_State* game_state)
{
	Vector2 previous_position = game_state->previous_tetromino_position;
	Vector2 current_position = game_state->current_tetromino.pivot_position; 
	uint8_t current_rotation = game_state->current_tetromino.rotation;
	uint8_t previous_rotation = game_state->previous_tetromino_rotation;

	if (game_state->should_spawn_tetromino)
	{
		return true;
	}

	if (will_fall_this_turn(game_state) ||
		current_position.x != previous_position.x || 
		current_position.y != previous_position.y || 
		current_rotation != previous_rotation)
	{
		enum Tetromino_Type type = game_state->current_tetromino.type;
		
		delete_tetromino_from_board(game_state, type, previous_position.x, previous_position.y, previous_rotation);
		
		printf("Deleted Type: %i -- At: %i, %i -- Rotation: %i\n", type, previous_position.x, previous_position.y, previous_rotation);

		return true;
	}

	return false;
}

inline bool will_fall_this_turn(Game_State* game_state)
{
	return (game_state->fall_clock >= get_current_fall_time(game_state));
}

bool tetromino_fall(Game_State* game_state)
{
	// Clamp movement to avoid overflows or collisions:
	clamp_movement(game_state);
		
	game_state->fall_clock = 0.0f;
	game_state->current_tetromino.pivot_position.y--;

	if (!is_possible_movement(game_state, false))
	{
		printf("--- Falled ---\n");
		game_state->current_tetromino.pivot_position.y++;
		return false;
	}

	return true;	
}

void determine_current_destination(Game_State* game_state)
{
	int16_t initial_y = game_state->current_tetromino.pivot_position.y;
	int16_t y_offset = 0;

	while (is_possible_movement(game_state, false))
	{
		y_offset--;
		game_state->current_tetromino.pivot_position.y--;
	};

	y_offset++;	

	game_state->current_destination.y = initial_y + y_offset;
	game_state->current_destination.x = game_state->current_tetromino.pivot_position.x;
	game_state->current_tetromino.pivot_position.y = initial_y;

	printf("--- Determined Current Destination: (%i, %i) initial_y: %i y_offset: %i ---\n", game_state->current_destination.x, game_state->current_destination.y, initial_y, y_offset);
}

void check_game_over(Game_State* game_state)
{
	// If tetromino cannot fall any further, and its pivot is beyond rendered board this means user has lost the game:
	for (size_t j = BOARD_HEIGHT_RENDERED; j < BOARD_HEIGHT; ++j)
	{
		bool has_line = true;
		
		for (size_t i = 0; i < BOARD_WIDTH; ++i)
		{	
			enum Tetromino_Type tetromino_type = get_2d_array_element(game_state->board, BOARD_WIDTH, i, j);

			if (tetromino_type != EMPTY_CELL_TYPE)
			{
				game_state->game_phase = GAME_PHASE_GAMEOVER;
				return;
			}
		}
	}
}

void destroy_lines(Game_State* game_state)
{
	uint8_t line_count = 0;
	size_t new_board_index = 0;
	uint8_t new_board[BOARD_SIZE]; 

	memset(new_board, EMPTY_CELL_TYPE, BOARD_SIZE);

	for (size_t j = 0; j < BOARD_HEIGHT; ++j)
	{
		bool has_line = true;
		
		for (size_t i = 0; i < BOARD_WIDTH; ++i)
		{	
			enum Tetromino_Type tetromino_type = get_2d_array_element(game_state->board, BOARD_WIDTH, i, j);

			if (tetromino_type == EMPTY_CELL_TYPE)
			{
				has_line = false;
				break;
			}
		}

		if (has_line)
		{
			game_state->tetromino_lines[j] = DURATION_LINE_ANIMATION;
			line_count++;
			continue;
		}

		for (size_t i = 0; i < BOARD_WIDTH; ++i)
		{	
			enum Tetromino_Type tetromino_type = get_2d_array_element(game_state->board, BOARD_WIDTH, i, j);

			new_board[new_board_index + i] = tetromino_type;
		}

		new_board_index += BOARD_WIDTH;
	}

	for (size_t i = 0; i < BOARD_SIZE; ++i)
	{
		game_state->board[i] = new_board[i];
	}
	
	game_state->line_count += line_count;
}

void update_line_data(Game_State* game_state)
{
	float_t delta_time = game_state->delta_time;

	for (size_t i = 0; i < BOARD_HEIGHT_RENDERED; i++)
	{
		float_t remaining_duration = game_state->tetromino_lines[i];
		remaining_duration = max(0, remaining_duration - delta_time);
		game_state->tetromino_lines[i] = remaining_duration;
	}
}

void update_game_text(Game_State* game_state, Text_State* text_state)
{
	switch (game_state->game_phase)
	{
	case GAME_PHASE_PLAYING:
		snprintf(text_state->level_text.buffer, TEXT_BUFFER_SIZE, "LEVEL: %i", game_state->current_level);
		snprintf(text_state->line_text.buffer, TEXT_BUFFER_SIZE, "LINES: %i", game_state->line_count);
		snprintf(text_state->score_text.buffer, TEXT_BUFFER_SIZE, "SCORE: %i", game_state->score);
		break;
	default:
		break;
	}
}

void update_game_playing_phase(Game_State* game_state, Input_State* input_state)
{
	if (game_state->should_spawn_tetromino)
	{
		enum Tetromino_Type initial_tetromino_type = (enum Tetromino_Type)random_range(0, TETROMINO_TYPE_COUNT-1);

		printf("Generated new tetromino of type: %i\n", initial_tetromino_type);

		Vector2 spawn_position = {.x = 4, .y = 20};

		Tetromino new_tetromino = {
			.pivot_position = spawn_position,
			.rotation = 0,
			.type = initial_tetromino_type,
		};

		// Set initial state:
		game_state->current_tetromino = new_tetromino;
		game_state->previous_tetromino_position = spawn_position;
		game_state->previous_tetromino_rotation = 0;
	}

	// Update line related data if any line is currently active:
	update_line_data(game_state);

	// Parse input commands:
	parse_input_state_playing_phase(game_state, input_state);

	// If created new and in illegal cell after parsing input, go to game over state:
	if (game_state->should_spawn_tetromino && !is_possible_movement(game_state, true))
	{
		game_state->game_phase = GAME_PHASE_GAMEOVER;
		return;
	}

	// Tick the fall clock:
	game_state->fall_clock += (float_t)game_state->delta_time;

	// Delete previous state of same tetromino if any feature is different from the previous state:
	bool recycled_tetromino = recycle_current_tetromino(game_state);
	
	// This flag is controlled by fall algorithm, which decides if the tetromino can fall any further:
	bool cannot_fall = false;
	
	// Decide if fall will occur this time:
	if (will_fall_this_turn(game_state))	
	{	
		// Try falling, get if fall was successfull:
		cannot_fall = !tetromino_fall(game_state);
	}

	// Cache tetromino memory location:
	Tetromino* current_tetromino = &(game_state->current_tetromino);
	
	// Move tetromino horizontally if colliding with borders of board:
	move_tetromino_for_rotation(game_state, game_state->should_spawn_tetromino);

	// Put the new state of tetromino to the board if previous was deleted or this is a new tetromino:
	if (recycled_tetromino || game_state->should_spawn_tetromino)
	{
		// Clamp movement to avoid overflows or collisions:
		clamp_movement(game_state);

		// Determine final possible final destination for currently falling tetromino:
		determine_current_destination(game_state);

		// Fill corresponding cells in board:
		put_tetromino_to_board(game_state, game_state->should_spawn_tetromino);
	}

	// Set previouses:
	// These are highly used for validation of any movement.
	game_state->previous_tetromino_position = current_tetromino->pivot_position;
	game_state->previous_tetromino_rotation = current_tetromino->rotation;

	// Set should_spawn_tetromino to cannot_fall so that new tetromino is spawned if the current one cannot fall any further:
	game_state->should_spawn_tetromino = cannot_fall;

	if (game_state->should_spawn_tetromino)
	{
		// Get line count before destroying new ones,
		// Destroy the lines if there are any,
		// Calculate new lines destroyed this frame,
		uint8_t new_line_count = game_state->line_count;
		destroy_lines(game_state);
		new_line_count = (game_state->line_count - new_line_count);

		// Add score using new lines:
		add_score(game_state, new_line_count);

		// Check for game over condition:
		check_game_over(game_state);

		// Level up if requirements are met:
		level_up(game_state);
	}
}

void update_game_gameover_phase(Game_State* game_state, Input_State* input_state)
{
	if (input_state->pressed_space)
	{
		// Reset game state:
		initialize_game_state(game_state);
	}
}

void update_game(Game_State* game_state, Input_State* input_state)
{	
	switch (game_state->game_phase)
	{
	case GAME_PHASE_PLAYING:
		update_game_playing_phase(game_state, input_state);
	break;
	
	case GAME_PHASE_GAMEOVER:
		update_game_gameover_phase(game_state, input_state);
	break;
	}

}

void initialize_game_state(Game_State* game_state)
{
	// Clear board to empty cells:
	memset(&game_state->board, EMPTY_CELL_TYPE, BOARD_SIZE);

	// Initialize lines to 0.0f:
	memset(&game_state->tetromino_lines, 0.0f, BOARD_HEIGHT_RENDERED);

	game_state->game_phase = GAME_PHASE_PLAYING;
	game_state->should_spawn_tetromino = true;  

	game_state->current_level = 0;
	game_state->line_count = 0;
	game_state->score = 0;
	
	// Delta time:
	game_state->delta_time = 0.0;

	game_state->current_destination = (Vector2) {.x = 0, .y = 0};
	game_state->fall_clock = 0.0f;
}

void initialize_game(Game_State* game_state, Input_State* input_state, Text_State* text_state)
{
	// Reset game_state related variables:
	initialize_game_state(game_state);	

	// Reset input variables:
	reset_input_state(input_state);

	// Initialize Texts:
	text_state->level_text.position = (Vector2){.x = SCREEN_WIDTH - TETROMINO_SIZE, .y = TETROMINO_SIZE};
	text_state->level_text.alignment = TEXT_ALIGNMENT_RIGHT;
	
	text_state->line_text.position = (Vector2){.x = SCREEN_WIDTH - TETROMINO_SIZE, .y = TETROMINO_SIZE * 1.5};
	text_state->line_text.alignment = TEXT_ALIGNMENT_RIGHT;
	
	text_state->score_text.position = (Vector2){.x = TETROMINO_SIZE, .y = TETROMINO_SIZE};
	text_state->score_text.alignment = TEXT_ALIGNMENT_LEFT;
}

void reset_input_state(Input_State* input_state)
{
	input_state->pressed_down = false;
	input_state->pressed_left = false;
	input_state->pressed_right = false;
	input_state->pressed_up = false;
	input_state->pressed_space = false;
}

void parse_input_state_playing_phase(Game_State* game_state, Input_State* input_state)
{
	if (input_state->pressed_right)
	{
		printf("INPUT: Pressed right\n");
		game_state->current_tetromino.pivot_position.x++;
	}

	if (input_state->pressed_left)
	{
		printf("INPUT: Pressed left\n");
		game_state->current_tetromino.pivot_position.x--;
	}

	if (input_state->pressed_up)
	{
		printf("INPUT: Pressed up\n");
		game_state->current_tetromino.rotation = (game_state->current_tetromino.rotation + 1) % TETROMINO_ROTATION_COUNT;
	}

	if (input_state->pressed_down)
	{
		printf("INPUT: Pressed down\n");
		// game_state->current_tetromino.rotation = (game_state->current_tetromino.rotation - 1 + TETROMINO_ROTATION_COUNT) % TETROMINO_ROTATION_COUNT;
		game_state->current_tetromino.pivot_position.y--;
	}

	if (input_state->pressed_space)
	{
		printf("INPUT: Pressed space\n");
	}
}

void render_game_text_playing_phase(Game_State* game_state, Text_State* text_state, SDL_Renderer* renderer, TTF_Font* font_24pt, TTF_Font* font_16pt)
{
	draw_text(renderer, font_16pt, text_state->score_text.buffer, text_state->score_text.position, text_state->score_text.alignment, TEXT_RENDER_MODE_BLENDED, LINE_COLOR);
	draw_text(renderer, font_16pt, text_state->line_text.buffer, text_state->line_text.position, text_state->line_text.alignment, TEXT_RENDER_MODE_BLENDED, LINE_COLOR);
	draw_text(renderer, font_16pt, text_state->level_text.buffer, text_state->level_text.position, text_state->level_text.alignment, TEXT_RENDER_MODE_BLENDED, LINE_COLOR);
}

void render_game_text_gameover_phase(Game_State* game_state, Text_State* text_state, SDL_Renderer* renderer, TTF_Font* font_24pt, TTF_Font* font_16pt)
{
	render_game_text_playing_phase(game_state, text_state, renderer, font_24pt, font_16pt);
	
	// Render gameover text over game phase text:
	draw_text(renderer, font_24pt, "GAME OVER", (Vector2){.x = SCREEN_WIDTH/2, .y = SCREEN_HEIGHT/2}, TEXT_ALIGNMENT_CENTER, TEXT_RENDER_MODE_BLENDED, LINE_COLOR);
	draw_text(renderer, font_16pt, "PRESS SPACE TO PLAY AGAIN",(Vector2) {.x = SCREEN_WIDTH/2, .y = (SCREEN_HEIGHT/2) + 32}, TEXT_ALIGNMENT_CENTER, TEXT_RENDER_MODE_BLENDED, LINE_COLOR);
}	

void render_game_text(Game_State* game_state, Text_State* text_state, SDL_Renderer* renderer, TTF_Font* font_24pt, TTF_Font* font_16pt)
{
	switch (game_state->game_phase)
	{
	case GAME_PHASE_PLAYING:
		render_game_text_playing_phase(game_state, text_state, renderer, font_24pt, font_16pt);
		break;
	case GAME_PHASE_GAMEOVER:
		render_game_text_gameover_phase(game_state, text_state, renderer, font_24pt, font_16pt);
	default:
		break;
	}
}

void draw_tetromino_unit(SDL_Renderer* renderer, int row, int column, enum Tetromino_Type type)
{
	int x_position = BOARD_OFFSET_X + (row * (TETROMINO_SIZE));
	int y_position = BOARD_OFFSET_Y + ((BOARD_HEIGHT - 1 - column) * (TETROMINO_SIZE));
		
	draw_filled_rectangle(renderer, x_position, y_position, TETROMINO_SIZE, TETROMINO_SIZE, COLORS[type][2]);
	draw_filled_rectangle(renderer, x_position + 3, y_position, TETROMINO_SIZE - 3, TETROMINO_SIZE - 3, COLORS[type][0]);
	draw_filled_rectangle(renderer, x_position + 3, y_position + 3, TETROMINO_SIZE - 6, TETROMINO_SIZE - 6, COLORS[type][1]);
}

void draw_empty_cell(SDL_Renderer* renderer, int row, int column)
{
	int x_position = BOARD_OFFSET_X + (row * (TETROMINO_SIZE));
	int y_position = BOARD_OFFSET_Y + ((BOARD_HEIGHT - 1 - column) * (TETROMINO_SIZE));

	draw_filled_rectangle(renderer, x_position + 3, y_position + 3, TETROMINO_SIZE - 6, TETROMINO_SIZE - 6, EMPTY_CELL_COLOR);
}

void draw_current_destination(Game_State* game_state, SDL_Renderer* renderer)
{
	if (game_state->game_phase == GAME_PHASE_GAMEOVER)
	{
		return;
	}

	Tetromino tetromino = (game_state->current_tetromino);
	Vector2 destination = game_state->current_destination;

	for (size_t i = 0; i < MAX_TETROMINO_WIDTH; ++i)
	{
		for (size_t j = 0; j < MAX_TETROMINO_HEIGHT; ++j)
		{
			uint8_t cell_value = TETROMINOES[tetromino.type][tetromino.rotation][j][i];

			if (cell_value == 0)
			{
				continue;
			}

			int16_t offset_x = i - TETROMINO_PIVOT_X;
			int16_t offset_y = j - TETROMINO_PIVOT_Y;
			int board_x = destination.x + offset_x;
			int board_y = destination.y - offset_y;
			
			int x_position = BOARD_OFFSET_X + (board_x * (TETROMINO_SIZE));
			int y_position = BOARD_OFFSET_Y + ((BOARD_HEIGHT - 1 - board_y) * (TETROMINO_SIZE));

			if (board_y >= BOARD_HEIGHT_RENDERED)
			{
				continue;
			}
			
			// draw_filled_rectangle(renderer, x_position, y_position, TETROMINO_SIZE, TETROMINO_SIZE, COLORS[tetromino.type][1]);
			draw_filled_rectangle(renderer, x_position + 3, y_position + 3, TETROMINO_SIZE - 6, TETROMINO_SIZE - 6, COLORS[tetromino.type][2]);
		}
	}
}

void draw_tetrominoes(Game_State* game_state, SDL_Renderer* renderer)
{
	for (size_t i = 0; i < BOARD_WIDTH; ++i)
	{
		for (size_t j = 0; j < BOARD_HEIGHT; ++j)
		{	
			uint8_t current_board_element_type = get_2d_array_element(game_state->board, BOARD_WIDTH, i,j); 

			if (current_board_element_type == EMPTY_CELL_TYPE)
			{
				continue;
			}

			draw_tetromino_unit(renderer, i, j, current_board_element_type);
		}
	}
}

void draw_board_cells(Game_State* game_state, SDL_Renderer* renderer)
{
	for (size_t i = 0; i < BOARD_WIDTH; ++i)
	{
		for (size_t j = 0; j < BOARD_HEIGHT_RENDERED; ++j)
		{	
			uint8_t current_board_element_type = get_2d_array_element(game_state->board, BOARD_WIDTH, i,j); 
			
			// Don't draw if cell is not empty:
			if (current_board_element_type != EMPTY_CELL_TYPE)
			{
				continue;
			}
		
			draw_empty_cell(renderer, i, j);
		}
	}
}

void draw_lines(Game_State* game_state, SDL_Renderer* renderer)
{
	for (size_t j = 0; j < BOARD_HEIGHT_RENDERED; ++j)
	{
		// Read line data from tetromino_lines, if it is 0, that means line on that row is not active:
		if (game_state->tetromino_lines[j] <= 0.0f)
		{
			continue;
		}

		// Draw squares on that row:
		for (size_t i = 0; i < BOARD_WIDTH; ++i)
		{
			int x_position = BOARD_OFFSET_X + (i * (TETROMINO_SIZE));
			int y_position = BOARD_OFFSET_Y + ((BOARD_HEIGHT - 1 - j) * (TETROMINO_SIZE));

			// Calculate scale by time remaining on this line:
			float_t scale = game_state->tetromino_lines[j] / DURATION_LINE_ANIMATION;
			int size = (int)((float_t)TETROMINO_SIZE * scale);
			int delta_half = (TETROMINO_SIZE - size) / 2;

			// Draw square using scaled values:
			draw_filled_rectangle(renderer, x_position + delta_half, y_position + delta_half, TETROMINO_SIZE - delta_half*2, TETROMINO_SIZE - delta_half*2, LINE_COLOR);
		}
	}
}

void render_game_playing_phase(Game_State* game_state, SDL_Renderer* renderer)
{
	// Draw empty cells:
	draw_board_cells(game_state, renderer);

	// Draw the final destination of tetromino:
	draw_current_destination(game_state, renderer);
	
	// Draw All Tetrominoes:
	draw_tetrominoes(game_state, renderer);

	// Draw Lines:
	draw_lines(game_state, renderer);
}

void render_game_gameover_phase(Game_State* game_state, SDL_Renderer* renderer)
{
	render_game_playing_phase(game_state, renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	draw_filled_rectangle(renderer, 0,0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color) {.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x80});
}

void render_game(Game_State* game_state, SDL_Renderer* renderer)
{
	switch (game_state->game_phase)
	{
	case GAME_PHASE_PLAYING:
		render_game_playing_phase(game_state, renderer);
	break;

	case GAME_PHASE_GAMEOVER:	
		render_game_gameover_phase(game_state, renderer);
	break;
	}
}	

