#ifdef WIN32
#include <windows.h>
#include <curses.h>
#define XSLEEP(time) Sleep(time)
#else
#include <ncurses.h>
#define XSLEEP(time) nanosleep((struct timespec[]){{0, (time * 1000000L)}}, NULL)
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QUAD_WIDTH 15
#define QUAD_HEIGHT 7
#define MEMCHAIN_LEN 1024
#define NUM_COLORS 4


// Initializes ncurses with some good settings.
void init_ncurses(void);

// A quadrant is just a colored section of the simon 'square'.
void draw_quadrant(const int startx, const int starty, const int colorpair);

// Draws 4 quadrants to make a board.
void draw_game_board(const int startx, const int starty, const int highlight);

// Draws the gameboard along with some other text.
void draw_screen(const int boardx, const int boardy, const int highlight, const int score, const char message[]);

/* Plays back the memory chain, highlighting each color quadrant. 
   This is done by drawing each link as a game board. */
void simon_says(const int memchain[], const int startx, const int starty, const int score);

// Adds a new random link in the memory chain.
void add_mem_link(int memchain[], const int score);

// Returns a random number between 1 and 4 which represents a color.
int get_random_color(void);

// Resets the game, basically by setting score and memchain to 0.
void reset_game(const int boardx, const int boardy, int * const score, int memchain[]);

// Does a little cleanup before exiting.
void exit_game(void);


int main()
{
    /* Holds each successive color the user must select to keep the chain going.
       Only holds a value of 0, 1, 2, 3 or 4 with 0 representing no value
       and 1-4 representing a color. */
    int memchain[MEMCHAIN_LEN] = {0};
    
    // Player's score is increased each time the memory chained is played back successfully.
    int score = 0;

    int rows, cols;

    int reset;  // 1 or 0, determines if game is about to be reset.

    // Initialize some stuff.
    srand(time(0));
    init_ncurses();
    getmaxyx(stdscr, rows, cols);

    // Used to center the gameboard on the screen.
    int boardx = (cols / 2) - QUAD_WIDTH;
    int boardy = (rows / 2) - QUAD_HEIGHT - 2;

    
    draw_screen(boardx, boardy, 0, score, "Press any key to start.");
    getch();
    draw_screen(boardx, boardy, 0, score, "Prepare yourself!");
    XSLEEP(999);

    while(1)
    {
        reset = 0;
        add_mem_link(memchain, score);
        simon_says(memchain, boardx, boardy, score);
        draw_screen(boardx, boardy, 0, score, "Do you remember?");

        for(int i = 0; i <= score; ++i)
        {
            // Need to flush the input or else any key the user pressed while waiting will be returned by getch().
            flushinp();

            // getch() - '0' will return the actual integer value of the key pressed.
            int keyp = getch() - '0';

            // If the player picked the incorrect color...
            if(keyp != memchain[i])
            {
                draw_screen(boardx, boardy, 0, score, "Game over. Press any key to play again. Press <Q> to quit.");
                keyp = getch();

                if(keyp == 'q')
                {
                    exit_game();
                    return 0;
                }
                else
                {
                    reset = 1;
                    reset_game(boardx, boardy, &score, memchain);
                    break;
                }
            }
            else
            {
                draw_screen(boardx, boardy, keyp, score, "");
                XSLEEP(200);
                draw_screen(boardx, boardy, 0, score, "");
            }
        }

        if(!reset)
        {
            draw_screen(boardx, boardy, 0, score, "Correct! Get ready...");

            ++score;
            XSLEEP(999);
        }
    }

    exit_game();

    return 0;
}


void init_ncurses(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    use_default_colors();  // Want to use default background color, '-1'.
    start_color();

    // The different simon colors.
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_BLUE, -1);
}


void draw_quadrant(const int startx, const int starty, const int colorpair)
{
    attron(COLOR_PAIR(colorpair));
    for(int x = 0; x < QUAD_WIDTH; ++x)
    {
        for(int y = 0; y < QUAD_HEIGHT; ++y)
            mvaddch(starty + y, startx + x, ACS_CKBOARD);
    }
    attroff(COLOR_PAIR(colorpair));
}


void draw_game_board(const int startx, const int starty, const int hightlight)
{
    // Loop through the 4 color quadrants.
    for(int i = 1; i <= NUM_COLORS; ++i)
    {
        // Quadrants 1 and 3 have the same startx, as well as 2 and 4.
        int qx = (i % 2) ? startx : startx + QUAD_WIDTH; 

        // Quadrants 1 and 2 have the same startx, as well as 3 and 4.
        int qy = (i <= 2) ? starty : starty + QUAD_HEIGHT; 

        // Highlight this quadrant to make it 'flash'.
        if(hightlight == i) attron(A_STANDOUT);
        draw_quadrant(qx, qy, i);
        if(hightlight == i) attroff(A_STANDOUT);
    }
}


void draw_screen(const int boardx, const int boardy, const int highlight, const int score, const char message[])
{
    clear();
    mvprintw(boardy - 1, boardx + 1, "nSimon v1.0");
    mvprintw(boardy - 1, boardx + QUAD_WIDTH + 3, "Score: %d", score);
    draw_game_board(boardx, boardy, highlight);
    mvprintw(boardy + QUAD_HEIGHT * 2, boardx + 4, "Press:");

    // Shows which key number is associated with which color.
    for(int i = 1; i <= NUM_COLORS; ++i)
    {
        attron(COLOR_PAIR(i));
        printw(" [%d]", i);
        attroff(COLOR_PAIR(i));
    }

    mvprintw(boardy + QUAD_HEIGHT * 2 + 2, (boardx + QUAD_WIDTH) - (strlen(message) / 2), message);

    refresh();
}


void simon_says(const int memchain[], const int startx, const int starty, const int score)
{
    // A value of 0 means no color, thus we haven't gotten to that point in the memchain yet.
    for(int i = 0; memchain[i] != 0; ++i)
    {
        // Draws a game board with the specified color highlighted.
        draw_screen(startx, starty, memchain[i], score, "");
        XSLEEP(500);  // Sleep briefly to let the user see.
        /* Do the above again but with no quadrant highlighted.
           We need this brief flash just in case the same quadrant is flashed twice in a row. */
        draw_screen(startx, starty, 0, score, "");
        XSLEEP(200);
    }
}


void add_mem_link(int memchain[], const int score)
{
    memchain[score] = get_random_color();
}


int get_random_color(void)
{
    return (rand() % NUM_COLORS) + 1;
}


void reset_game(const int boardx, const int boardy, int * const score, int memchain[])
{
    // Originally had *score + 1 instead of MEMCHAIN_LEN. Didn't work for some reason though.
    memset(memchain, 0, MEMCHAIN_LEN);
    *score = 0;
    draw_screen(boardx, boardy, 0, *score, "Prepare yourself!");
    XSLEEP(999);
}


void exit_game(void)
{
    // Maybe I'll need to do more in the future...
    endwin();
}
