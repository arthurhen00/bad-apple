#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FRAMES 6569
#define FPS 30
#define FRAME_TIME_MS (1000.0 / FPS)

#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>

#define FRAME_BUFFER_SIZE 65536

static char frame_buffer[FRAME_BUFFER_SIZE];


/* ============================================================
 * WEBASSEMBLY
 * ============================================================ */

/*
 * Carrega um frame específico.
 *
 * O JavaScript chama:
 *
 *     Module._get_frame(1)
 *
 * e recebe um ponteiro para o conteúdo de BA0001.txt.
 */
EMSCRIPTEN_KEEPALIVE
const char *get_frame(int frame) {
    if (frame < 1 || frame > FRAMES) {
        return NULL;
    }

    char file_path[32];

    snprintf(file_path, sizeof(file_path), "frames/BA%04d.txt", frame);

    FILE *file = fopen(file_path, "r");

    if (file == NULL) {
        return NULL;
    }

    size_t size = fread(frame_buffer, 1, FRAME_BUFFER_SIZE - 1, file);

    fclose(file);

    frame_buffer[size] = '\0';

    return frame_buffer;
}


/*
 * Retorna a quantidade de frames.
 */
EMSCRIPTEN_KEEPALIVE
int get_frame_count(void) {
    return FRAMES;
}


/*
 * Retorna o FPS.
 */
EMSCRIPTEN_KEEPALIVE
int get_fps(void) {
    return FPS;
}


#else


/* ============================================================
 * LINUX
 * ============================================================ */

#include <unistd.h>


double elapsed_ms(struct timespec *start) {
    struct timespec current;

    clock_gettime(CLOCK_MONOTONIC, &current);

    double seconds = (double)(current.tv_sec - start->tv_sec);
    double nanoseconds = (double)(current.tv_nsec - start->tv_nsec);

    return seconds * 1000.0 + nanoseconds / 1000000.0;
}


void csrs(void) {
    printf("\033[1;1H");
}


void clear_terminal(void) {
    printf("\033[2J\033[1;1H");
}


void play_audio(void) {
    system("aplay -q ./frames/BA.wav &");
}


/* ============================================================
 * MAIN
 * ============================================================ */

int main(void) {
    size_t nread;
    char buf[3072];

    printf("press any button to start\n");
    getchar();

    clear_terminal();

    play_audio();

    struct timespec start;

    clock_gettime(CLOCK_MONOTONIC, &start);


    for (int i = 1; i <= FRAMES; i++) {
        /*
         * Calcula o instante ideal do frame.
         *
         * Frame 1 -> 0 ms
         * Frame 2 -> 33.33 ms
         * Frame 3 -> 66.66 ms
         * ...
         */
        double target_ms = (i - 1) * FRAME_TIME_MS;

        while (elapsed_ms(&start) < target_ms) {
            usleep(1000);
        }

        char file_path[32];
        snprintf(file_path, sizeof(file_path), "./frames/BA%04d.txt", i);

        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            fprintf(stderr, "\nErro ao abrir frame: %s\n", file_path);
            break;
        }

        while ((nread = fread(buf, 1, sizeof(buf), file)) > 0) {
            fwrite(buf, 1, nread, stdout);
        }

        fclose(file);


        /*
         * HUD.
         */
        double current_ms = elapsed_ms(&start);
        int time_sec = (int)(current_ms / 1000.0);
        int min = time_sec / 60;
        int sec = time_sec % 60;

        printf("\n ------------------------\n");
        printf(" | Time: %02d:%02d          |\n", min, sec);
        printf(" | Frame: %04d          |", i);
        printf("\n ------------------------\n");

        csrs();

        fflush(stdout);
    }

    clear_terminal();

    printf("press a button to quit\n");

    getchar();

    return 0;
}

#endif