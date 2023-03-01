#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>

/* Define window resolutions */
#define START_WIDTH 480
#define START_HEIGHT 640

/* Define the scale */
#define SCALE 10

/* Define tile sizes */
#define TILE_WIDTH 8
#define TILE_HEIGHT 5


/* Define the character's attributes */
#define PLAYER_WIDTH 9
#define PLAYER_HEIGHT 11
#define PLAYER_STEP 2

/* Variables */
int EXIT_CODE = 0;
int screen_width;
int screen_height;
int playerX = 0;
int playerY = 0;
int backX = 0;
int backY = 0;
SDL_RendererFlip playerDirection;
SDL_RendererFlip backDirection;
int eaten [ ( 3840 / ( TILE_WIDTH * SCALE ) ) ][ ( 2160 / ( TILE_HEIGHT * SCALE ) ) ] = { { 0 } };

/*
Log an SDL error with some error message to the output stream of our choice
@param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError( char *msg, const char *error ) {
	printf( "%s error: %s\n", msg, error );
}

/*
Loads a BMP image into a texture on the rendering device
@param file The image file to load
@param ren The renderer to load the texture onto
@return the loaded texture, or NULL if something went wrong.
*/
SDL_Texture* loadTexture( char *file, SDL_Renderer *ren ) {
	/* Load the image */
	SDL_Texture *texture = IMG_LoadTexture( ren, file );

	if ( texture == NULL ) {
		logSDLError( "LoadTexture", IMG_GetError() );
	}
	return texture;
}

/*
Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
width and height
@param tex The source texture we want to draw
@param ren The renderer we want to draw to
@param x The x coordinate to draw to
@param y The y coordinate to draw to
@param w The width of the texture to draw
@param h The height of the texture to draw
*/
void renderTexture( SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h, SDL_RendererFlip flip  ){
	/* Setup the destination rectangle to be at the position we want */
	SDL_Rect dst;

	if ( w == 0 ) {
		SDL_QueryTexture(tex, NULL, NULL, &w, NULL);
	}
	if ( h == 0 ) {
		SDL_QueryTexture(tex, NULL, NULL, NULL, &h);
	}

	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopyEx( ren, tex, NULL, &dst, 0, NULL, flip );
}

int inSquare( int rootX, int rootY, int checkX, int checkY, int lengthX, int lengthY ) {
	if (
	checkX - rootX < lengthX &&
	checkX - rootX > -lengthX &&
	checkY - rootY < lengthY &&
	checkY - rootY > -lengthY
	) return 1;
	return 0;
}
	
int main( int argc, char **argv ) {

	/* Initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) != 0 ) {
		logSDLError( "SDL_Init" , SDL_GetError() );
		EXIT_CODE = 1;
		goto quit_1;
	}
	
	/* Initialize SDL_image */
	if ( ( IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG ) != IMG_INIT_PNG ){
		logSDLError( "IMG_Init", IMG_GetError() );
		EXIT_CODE = 1;
		goto quit_1;
	}

	/* Initialize SDL_mixer */
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
		logSDLError( "Mix_OpenAudio", Mix_GetError() );
		EXIT_CODE = 1;
		goto quit_1;
	}
	
	/* Create Window */
	SDL_Window *window = SDL_CreateWindow( "Goat Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, START_WIDTH, START_HEIGHT, SDL_WINDOW_RESIZABLE );
	if ( window == NULL ) {
		logSDLError( "CreateWindow", SDL_GetError() );
		EXIT_CODE = 2;
		goto quit_2;
	}

	/* Create Renderer */
	SDL_Renderer *renderer = SDL_CreateRenderer( window,  -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if ( renderer == NULL ) {
		logSDLError( "SDL_CreateRenderer", SDL_GetError() );
		EXIT_CODE = 3;
		goto quit_3;
	}

	printf( "Initialization Successfull\n" );

	/* Game Assets */
	SDL_Texture *grass = loadTexture( "img/grass.png" , renderer );
	SDL_Texture *bush = loadTexture( "img/bush.png" , renderer );
	SDL_Texture *goat = loadTexture( "img/goat.png" , renderer );
	if ( grass == NULL || bush == NULL || goat == NULL ) {
		logSDLError( "Image loading", IMG_GetError() );
		EXIT_CODE = 4;
		goto quit_4;
	}
	
	/* Add the sound */
	Mix_Chunk *nom = Mix_LoadWAV( "sfx/nom.wav" );
	if ( nom == NULL ) {
		logSDLError( "Sound loading", Mix_GetError() );
		EXIT_CODE = 5;
		goto quit_5;
	}

	printf( "Game Assets Loaded\n" );

	/* Game Code */
	
	SDL_Event e;
	int quit = 0;
	while ( quit == 0 ) {
		while ( SDL_PollEvent(&e) ) {
			if ( e.type == SDL_QUIT ) {
				quit = 1;
			}
			if ( e.type == SDL_KEYDOWN ) {
				switch ( e.key.keysym.sym ) {

					/* Mission Critical */

					/* Q (for quit) */
					case SDLK_q:
					quit = 1;
					break;
					
					/* ESCAPE (for quit) */
					case SDLK_ESCAPE:
					quit = 1;
					break;
					
					/* -- Player Movement -- */

					/* Right Arrow */
					case SDLK_RIGHT:
					playerX++;
					playerDirection = SDL_FLIP_HORIZONTAL;
					break;
					
					/* Left Arrow */
					case SDLK_LEFT:
					playerX--;
					playerDirection = SDL_FLIP_NONE;
					break;

					/* Up Arrow */
					case SDLK_UP:
					playerY--;
					break;

					/* Down Arrow */
					case SDLK_DOWN:
					playerY++;
					break;
					
					/* Space */
					case SDLK_SPACE:
					eaten[(playerX*PLAYER_STEP)/TILE_WIDTH][(playerY*PLAYER_STEP)/TILE_HEIGHT] = 1;
					Mix_PlayChannel( -1, nom, 0 );
					break;

					/* -- Background Movement -- */
		
					/* D */
					case SDLK_d:
					backX++;
					backDirection = SDL_FLIP_HORIZONTAL;
					break;
					
					/* A */
					case SDLK_a:
					backX--;
					backDirection = SDL_FLIP_NONE;
					break;

					/* W */
					case SDLK_w:
					backY--;
					break;

					/* S */
					case SDLK_s:
					backY++;
					break;

				}

			}
			if ( e.type == SDL_MOUSEBUTTONDOWN ) {
				quit = 1;
			}
		}

		/* Clear the renderer */
		SDL_RenderClear( renderer );
	
		/* Get screen size */
		SDL_GL_GetDrawableSize( window, &screen_width, &screen_height );

		if ( screen_width > 3840 || screen_height > 2160 ) {
			printf("Maximum screen dimesions exceeded.");
			break;
		}

		/* Tile the background */
		for (int x = 0; x * TILE_WIDTH * SCALE <= screen_width; x++ ) {
			for (int y = 0; y * TILE_HEIGHT * SCALE <= screen_height; y++ ) {
				renderTexture( grass, renderer,
				x * TILE_WIDTH * SCALE,
				y * TILE_HEIGHT * SCALE,
				TILE_WIDTH*SCALE, TILE_HEIGHT*SCALE,
				SDL_FLIP_NONE );
				
				int timeToBush = (( x + y ) % 2 == 0);
				if ( timeToBush && eaten[x][y] == 0 ) { /* Time to draw bush and bush is not eaten by goat */
					renderTexture( bush, renderer,
					((x * TILE_WIDTH) + ((backX % TILE_WIDTH) * PLAYER_STEP)) * SCALE,
					((y * TILE_HEIGHT) + ((backY % TILE_HEIGHT) * PLAYER_STEP)) * SCALE,
					TILE_WIDTH*SCALE, TILE_HEIGHT*SCALE, backDirection );
				}
			}
		}

		/* Draw the player */
		renderTexture(goat, renderer,
		SCALE * playerX * PLAYER_STEP,
		SCALE * playerY * PLAYER_STEP,
		PLAYER_WIDTH*SCALE, PLAYER_HEIGHT*SCALE, playerDirection );

		/* Update the screen */
		SDL_RenderPresent( renderer );

		/* Take a quick break after all that hard work */
		SDL_Delay( 33 );

	}
	
	/* Free sound effects */
	quit_5: Mix_FreeChunk( nom );
	nom = NULL;
	
	/* Free loaded images */
        quit_4: SDL_DestroyTexture(grass);
        SDL_DestroyTexture(bush);
        SDL_DestroyTexture(goat);
        grass = NULL;
        bush = NULL;
        goat = NULL;

        /* Destroy renderer */
	quit_3: SDL_DestroyRenderer( renderer );
	renderer = NULL;

	/* Destroy window */
	quit_2: SDL_DestroyWindow( window );
	window = NULL;

	/* Quit SDL subsystems */
	quit_1: Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	return EXIT_CODE;
}
