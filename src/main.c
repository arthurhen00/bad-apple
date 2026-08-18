#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define FRAMES 6569
#define FPS 30
#define FRAME_TIME_MS (1000.0 / FPS)

#define FRAME_FILE "frames/frames.bin"

#define FRAME_MAGIC "BAAS"
#define FRAME_VERSION 1

#define HEADER_SIZE 12
#define INDEX_ENTRY_SIZE 8

typedef struct {
    uint32_t offset;
    uint32_t size;
} FrameIndex;


#ifdef __EMSCRIPTEN__

#include <emscripten/emscripten.h>

#define FRAME_BUFFER_SIZE 65536

static char frame_buffer[FRAME_BUFFER_SIZE];

static FILE *frames_file = NULL;
static FrameIndex *frame_index = NULL;


/* ============================================================
 * WEBASSEMBLY
 * ============================================================ */

/*
 * Abre o arquivo binário e carrega o índice dos frames.
 */
static int frames_open(void) {
    frames_file = fopen(FRAME_FILE, "rb");

    if (frames_file == NULL) {
        return 0;
    }

    char magic[4];

    uint32_t version;
    uint32_t frame_count;


    /*
     * Header.
     *
     * magic       -> 4 bytes
     * version     -> 4 bytes
     * frame_count -> 4 bytes
     */
    if (fread(magic, 1, 4, frames_file) != 4) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(&version, sizeof(uint32_t), 1, frames_file) != 1) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(&frame_count, sizeof(uint32_t), 1, frames_file) != 1) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (memcmp(magic, FRAME_MAGIC, 4) != 0) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (version != FRAME_VERSION) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (frame_count != FRAMES) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    /*
     * Carrega o índice inteiro para memória.
     *
     * Cada frame possui:
     *
     *     offset -> posição do frame no arquivo
     *     size   -> tamanho do frame
     */
    frame_index = malloc(sizeof(FrameIndex) * FRAMES);

    if (frame_index == NULL) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(frame_index, sizeof(FrameIndex), FRAMES, frames_file) != FRAMES) {
        free(frame_index);
        frame_index = NULL;

        fclose(frames_file);
        frames_file = NULL;

        return 0;
    }

    return 1;
}


/*
 * Fecha o arquivo e libera o índice.
 */
static void frames_close(void) {
    if (frames_file != NULL) {
        fclose(frames_file);
        frames_file = NULL;
    }

    free(frame_index);
    frame_index = NULL;
}


/*
 * Carrega um frame específico.
 *
 * O JavaScript chama:
 *
 *     Module._get_frame(1)
 *
 * e recebe um ponteiro para o conteúdo do frame 1.
 */
EMSCRIPTEN_KEEPALIVE
const char *get_frame(int frame) {
    if (frame < 1 || frame > FRAMES) {
        return NULL;
    }

    if (frames_file == NULL || frame_index == NULL) {
        /*
        * No WebAssembly não temos um main()
        * para inicializar o arquivo.
        *
        * Portanto inicializamos o frames.bin
        * na primeira chamada de get_frame().
        */
        if (!frames_open()) {
            return NULL;
        }
    }

    FrameIndex index = frame_index[frame - 1];

    /*
     * O buffer precisa comportar o frame inteiro.
     */
    if (index.size >= FRAME_BUFFER_SIZE) {
        return NULL;
    }

    /*
     * Vai diretamente para o frame no arquivo.
     */
    if (fseek(frames_file, index.offset, SEEK_SET) != 0) {
        return NULL;
    }

    /*
     * Lê o frame para o buffer reutilizável.
     */
    size_t size = fread(frame_buffer, 1, index.size, frames_file);

    if (size != index.size) {
        return NULL;
    }

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


static FILE *frames_file = NULL;
static FrameIndex *frame_index = NULL;

static char *frame_buffer = NULL;
static uint32_t frame_buffer_size = 0;


/*
 * Abre o arquivo binário e carrega o índice dos frames.
 */
static int frames_open(void) {
    frames_file = fopen(FRAME_FILE, "rb");

    if (frames_file == NULL) {
        return 0;
    }

    char magic[4];

    uint32_t version;
    uint32_t frame_count;

    /*
     * Header.
     *
     * magic       -> 4 bytes
     * version     -> 4 bytes
     * frame_count -> 4 bytes
     */
    if (fread(magic, 1, 4, frames_file) != 4) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(&version, sizeof(uint32_t), 1, frames_file) != 1) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(&frame_count, sizeof(uint32_t), 1, frames_file) != 1) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    /*
     * Valida o arquivo.
     */
    if (memcmp(magic, FRAME_MAGIC, 4) != 0) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (version != FRAME_VERSION) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (frame_count != FRAMES) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    /*
     * Carrega o índice inteiro para memória.
     */
    frame_index = malloc(sizeof(FrameIndex) * FRAMES);

    if (frame_index == NULL) {
        fclose(frames_file);
        frames_file = NULL;
        return 0;
    }

    if (fread(frame_index, sizeof(FrameIndex), FRAMES, frames_file) != FRAMES) {
        free(frame_index);
        frame_index = NULL;

        fclose(frames_file);
        frames_file = NULL;

        return 0;
    }

    /*
     * O maior frame determina o tamanho inicial
     * do buffer.
     */
    uint32_t max_size = 0;

    for (int i = 0; i < FRAMES; i++) {
        if (frame_index[i].size > max_size) {
            max_size = frame_index[i].size;
        }
    }

    frame_buffer_size = max_size + 1;

    frame_buffer = malloc(frame_buffer_size);

    if (frame_buffer == NULL) {
        free(frame_index);
        frame_index = NULL;

        fclose(frames_file);
        frames_file = NULL;

        return 0;
    }

    return 1;
}


/*
 * Fecha o arquivo e libera os recursos.
 */
static void frames_close(void) {
    if (frames_file != NULL) {
        fclose(frames_file);
        frames_file = NULL;
    }

    free(frame_index);
    frame_index = NULL;

    free(frame_buffer);
    frame_buffer = NULL;

    frame_buffer_size = 0;
}


/*
 * Carrega um frame específico para o buffer reutilizável.
 */
static int frame_read(int frame, char **buffer, uint32_t *size) {
    if (frame < 1 || frame > FRAMES) {
        return 0;
    }

    if (frames_file == NULL || frame_index == NULL || frame_buffer == NULL) {
        return 0;
    }

    FrameIndex index = frame_index[frame - 1];

    /*
     * Vai diretamente para o frame no arquivo.
     */
    if (fseek(frames_file, index.offset, SEEK_SET) != 0) {
        return 0;
    }

    /*
     * Lê o frame para o buffer reutilizável.
     */
    size_t read_size = fread(frame_buffer, 1, index.size, frames_file);

    if (read_size != index.size) {
        return 0;
    }

    frame_buffer[read_size] = '\0';

    *buffer = frame_buffer;
    *size = index.size;

    return 1;
}


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
    printf("press any button to start\n");
    getchar();

    clear_terminal();

    /*
     * Abre o arquivo binário e carrega o índice.
     */
    if (!frames_open()) {
        fprintf(stderr, "Erro ao abrir frames.bin\n");

        return 1;
    }

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

        /*
         * Carrega o frame diretamente do
         * arquivo binário.
         */
        char *frame_data;
        uint32_t frame_size;

        if (!frame_read(i, &frame_data, &frame_size)) {
            fprintf(stderr, "\nErro ao ler frame: %04d\n", i);
            break;
        }

        fwrite(frame_data, 1, frame_size, stdout);

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


    frames_close();

    clear_terminal();

    printf("press a button to quit\n");

    getchar();

    return 0;
}

#endif