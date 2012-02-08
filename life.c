#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef enum {CELL_DEAD, CELL_ALIVE} cell_t;
typedef struct {
	int rows;
	int cols;
	cell_t **cells;
} screen_t;

screen_t *NewScreen(int rows, int cols);
void DelScreen(screen_t *screen);

void FillScreen(screen_t *screen, int cells);
void NextScreen(screen_t *oldScreen, screen_t *newScreen);
void DrawScreen(WINDOW *win, screen_t *screen);

int main()
{
	screen_t *currentScreen;
	screen_t *newScreen;
	screen_t *tmpScreen;
	int ch, generation;
	
	initscr();
	//raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	/* insert random seed, needed for FillScreen() */
	srand(time(NULL));

	currentScreen = NewScreen(LINES, COLS);
	newScreen     = NewScreen(LINES, COLS);
	
	FillScreen(currentScreen, COLS*3 + rand() % 500);
	for (generation=1; ; generation++) {
		DrawScreen(stdscr, currentScreen);

		ch = 0;
		while (ch != KEY_LEFT) {
			ch = getch();

			if (ch == 'q') {
				goto end;
			}
		}

		NextScreen(currentScreen, newScreen);

		/* swap the screens */
		tmpScreen = currentScreen;
		currentScreen = newScreen;
		newScreen = tmpScreen;
		usleep(100);
	}

end:
	DelScreen(currentScreen);
	DelScreen(newScreen);
	
	endwin();
	return 0;
}

screen_t *NewScreen(int rows, int cols)
{
	screen_t *screen;
	cell_t **cells;
	int i;

	/* This can't be right?... */
	cells = (cell_t**)(malloc(sizeof(cell_t*)*rows));
	for (i=0; i < rows; i++) {
		cells[i] = (cell_t*)(malloc(sizeof(cell_t)*cols));
	}
	
	screen = (screen_t*)(malloc(sizeof(screen_t)));
	screen->rows  = rows;
	screen->cols  = cols;
	screen->cells = cells;

	return screen;
}

void DelScreen(screen_t *screen)
{
	free(screen->cells);
	free(screen);
}

void FillScreen(screen_t *screen, int cells)
{
	int i,j;

	/* clear screen first */
	for (i=0; i < screen->rows; i++) {
		for (j=0; j < screen->cols; j++) {
			screen->cells[i][j] = CELL_DEAD;
		}
	}

	/* draw random characters */
	while (cells-- > 0) {
		i = rand() % screen->rows;
		j = rand() % screen->cols;

		/* check if it's alive already */
		if (screen->cells[i][j] == CELL_ALIVE) {
			cells++;
		}

		screen->cells[i][j] = CELL_ALIVE;
	}
}

void NextScreen(screen_t *oldScreen, screen_t *newScreen)
{
	int i,j,k,l;
	int neighbours;

	for (i=0; i < oldScreen->rows; i++) {
		for (j=0; j < oldScreen->cols; j++) {
			neighbours = 0;
			for (k=-1; k < 2; k++) {
				for (l=-1; l < 2; l++) {
					if (i+k >= 0 && i+k < oldScreen->rows
					 && j+l >= 0 && j+l < oldScreen->cols
					 && oldScreen->cells[i+k][j+l] == CELL_ALIVE) {
						neighbours++;
					}
				}
			}
			/* don't count yourself  */
			if (oldScreen->cells[i][j] == CELL_ALIVE)
				neighbours--;

			if (neighbours == 3) {
				newScreen->cells[i][j] = CELL_ALIVE;
			}
			else if (neighbours == 2 && oldScreen->cells[i][j] == CELL_ALIVE) {
				newScreen->cells[i][j] = CELL_ALIVE;
			}
			else {
				newScreen->cells[i][j] = CELL_DEAD;
			}
		}
	}
}

void DrawScreen(WINDOW *win, screen_t *screen)
{
	int i,j;

	for (i=0; i < screen->rows; i++) {
		for (j=0; j < screen->cols; j++) {
			if (screen->cells[i][j] == CELL_ALIVE) {
				mvwaddch(win, i, j, '@');
			} else {
				mvwaddch(win, i, j, ' ');
			}
		}
	}
	wrefresh(win);
}
