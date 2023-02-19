#include <stdio.h>
#include <windows.h>
#include <time.h>

#define FRAMES 6569
#define THIRTY_FPS 30

void csrs(void);

int main(){

    size_t nread;
    char buf[3072];
    int timeInFrame;

	system("mode con: cols=120 lines=40");

    printf("press any button to start\n");
    getchar();
    system("cls");

    clock_t start_t, curr_t;
    int time_sec, count_t, min = 0, sec;

    PlaySound("./frames/BA.wav", NULL, SND_ASYNC);

    start_t = clock(); //starting the timer
	count_t = start_t;

    for(int i = 1; i <= FRAMES; i++){

        curr_t = clock();
		time_sec = (curr_t - start_t) / CLOCKS_PER_SEC;
		min = time_sec/60;
		sec = time_sec%60; //acquiring elapsed time in mm:ss
        
        char file_path[24];
        sprintf(file_path, "./frames/BA%d.txt", i);

        if(i % 30 == 0){
            timeInFrame = 43;
        } else {
            timeInFrame = 33;
        }

        if((curr_t - count_t) >= timeInFrame){
            FILE *arq = fopen(file_path, "r");
            while((nread = fread(buf, 1, sizeof buf, arq)) > 0){
                fwrite(buf, 1, nread, stdout);
            }
            fclose(arq);

            printf("\n ------------------------\n");
            printf(" | Time: %02d:%02d          |\n | Frame: %04d          |",min, sec, i);
            printf("\n ------------------------\n");
            csrs();
            count_t += timeInFrame;
        } else {
            i--;
        }
    }
    
    system("cls");
    printf("press a button to quit");
    getchar();

    return 0;
}

void csrs(void) //resetting the cursor
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { 0, 1 };
	SetConsoleCursorPosition(h,pos);
}