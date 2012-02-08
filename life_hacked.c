#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* globals */
static int cols;
static int rows;
static int dcols;
static int drows;

/*typedef enum {CELL_DEAD, CELL_ALIVE} cell_t;*/
typedef char cell_t;
typedef struct {
	int rows;
	int cols;
	cell_t **cells;
} screen_t;

screen_t *NewScreen(int rows, int cols);
void DelScreen(screen_t *screen);

void CopyScreen(screen_t *dst, screen_t *source);
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

	/* initialise bad globals */
	cols = COLS*3;
	rows = LINES*3;
	dcols = COLS;
	drows = LINES;

	/* initialise the nice colors */
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_GREEN, COLOR_BLACK);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_CYAN, COLOR_BLACK);
	init_pair(7, COLOR_RED, COLOR_BLACK);

	/* insert random seed, needed for FillScreen() */
	srand(time(NULL));

	currentScreen = NewScreen(rows, cols);
	newScreen     = NewScreen(rows, cols);
	
start:
	FillScreen(currentScreen, cols * rows / 5.0);
	for (generation=1; ; generation++) {
		DrawScreen(stdscr, currentScreen);

		ch = 0;
		while (1) {
			ch = getch();

			if (ch == 'n') {
				NextScreen(currentScreen, newScreen);
		                tmpScreen = currentScreen;
                		currentScreen = newScreen;
		                newScreen = tmpScreen;
				break;
			} else if (ch == KEY_LEFT && dcols-5 >= 0) {
				dcols -= 5;
				break;
			} else if (ch == KEY_LEFT) {
				dcols = 0;
				break;
			} else if (ch == KEY_RIGHT && dcols+5 < cols-COLS) {
				dcols += 5;
				break;
			} else if (ch == KEY_RIGHT) {
				dcols = cols - COLS;
				break;
			} else if (ch == KEY_UP && drows-5 >= 0) {
				drows -= 5;
				break;
			} else if (ch == KEY_UP) {
				drows = 0;
				break;
			} else if (ch == KEY_DOWN && drows+5 < rows-LINES) {
				drows += 5;
				break;
			} else if (ch == KEY_DOWN) {
				drows = rows - LINES;
				break;
			} else if (ch == KEY_PPAGE) {
				drows = 0;
				break;
			} else if (ch == KEY_NPAGE) {
				drows = rows - LINES;
				break;
			} else if (ch == KEY_HOME) {
				dcols = 0;
				break;
			} else if (ch == KEY_END) {
				dcols = cols - COLS;
				break;
			} else if (ch == 'q') {
				goto end;
			}
		}
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

void CopyScreen(screen_t *dst, screen_t *source)
{
	int i,j;

	if (dst->cols < source->cols || dst->rows < source->rows) {
		return;
	}

	for (i=0; i < source->rows; i++) {
		for (j=0; j < source->cols; j++) {
			dst->cells[i][j] = source->cells[i][j];
		}
	}
}

void FillScreen(screen_t *screen, int cells)
{
	int i,j;

	/* clear screen first */
	for (i=0; i < screen->rows; i++) {
		for (j=0; j < screen->cols; j++) {
			screen->cells[i][j] = 0;
		}
	}

	/* draw random characters */
	while (cells-- > 0) {
		i = rand() % screen->rows;
		j = rand() % screen->cols;

		/* check if it's alive already */
		if (screen->cells[i][j] > 0) {
			cells++;
		}
		
		screen->cells[i][j] = 1;
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
					 && oldScreen->cells[i+k][j+l] > 0) {
						neighbours++;
					}
				}
			}
			/* don't count yourself  */
			if (oldScreen->cells[i][j] > 0) {
				neighbours--;
			}

			if (neighbours == 3 && oldScreen->cells[i][j] == 0) {
				newScreen->cells[i][j] = 1;
			}
			else if ((neighbours == 2 || neighbours == 3) && oldScreen->cells[i][j] > 0) {
				newScreen->cells[i][j] = oldScreen->cells[i][j]+1;
			}
			else {
				newScreen->cells[i][j] = 0;
			}

			if (newScreen->cells[i][j] > 7) {
				newScreen->cells[i][j] = 7;
			}
		}
	}
}

void DrawScreen(WINDOW *win, screen_t *screen)
{
	int i,j;

	/*
	for (i=0; i < screen->rows; i++) {
		for (j=0; j < screen->cols; j++) {
			if (screen->cells[i][j] > 0) {
				attron(COLOR_PAIR(screen->cells[i][j]));
				mvwaddch(win, i, j, '@');
				attroff(COLOR_PAIR(screen->cells[i][j]));
			} else {
				mvwaddch(win, i, j, ' ');
			}
		}
	}
	*/
        for (i=0; i < LINES; i++) {
                for (j=0; j < COLS; j++) {
                        if (screen->cells[drows+i][dcols+j] > 0) {
                                attron(COLOR_PAIR(screen->cells[drows+i][dcols+j]));
                                mvwaddch(win, i, j, '@');
                                attroff(COLOR_PAIR(screen->cells[drows+i][dcols+j]));
                        } else {
                                mvwaddch(win, i, j, ' ');
                        }
                }
        }
	wrefresh(win);
}
