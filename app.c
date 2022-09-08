#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"

#define KEY_ESC     0x1B
#define KEY_ENTER   0x0A

#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 1
#define TASK3_PRIORITY 1
#define TASK4_PRIORITY 1

#define BLACK "\033[30m" /* Black */
#define RED "\033[31m"   /* Red */
#define GREEN "\033[32m" /* Green */
#define DISABLE_CURSOR() printf("\e[?25l")
#define ENABLE_CURSOR() printf("\e[?25h")

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))

char morseCode[36][6] = { 
    { '.', '-',   0,   0,   0,   0 }, // A
    { '-', '.', '.', '.',   0,   0 }, // B
    { '-', '.', '-', '.',   0,   0 }, // C
    { '-', '.', '.',   0,   0,   0 }, // D
    { '.',   0,   0,   0,   0,   0 }, // E
    { '.', '.', '-', '.',   0,   0 }, // F
    { '-', '-', '.',   0,   0,   0 }, // G
    { '.', '.', '.', '.',   0,   0 }, // H
    { '.', '.',   0,   0,   0,   0 }, // I
    { '.', '-', '-', '-',   0,   0 }, // J
    { '-', '.', '-',   0,   0,   0 }, // K
    { '.', '-', '.', '.',   0,   0 }, // L
    { '-', '-',   0,   0,   0,   0 }, // M
    { '-', '.',   0,   0,   0,   0 }, // N
    { '-', '-', '-',   0,   0,   0 }, // O
    { '.', '-', '-', '.',   0,   0 }, // P
    { '-', '-', '.', '_',   0,   0 }, // Q
    { '.', '-', '.',   0,   0,   0 }, // R
    { '.', '.', '.',   0,   0,   0 }, // S
    { '-',   0,   0,   0,   0,   0 }, // T
    { '.', '.', '-',   0,   0,   0 }, // U
    { '.', '-', '-',   0,   0,   0 }, // W
    { '-', '.', '.', '-',   0,   0 }, // X
    { '-', '.', '-', '-',   0,   0 }, // Y
    { '-', '-', '.', '.',   0,   0 }, // Z
    { '-', '-', '-', '-', '-',   0 }, // 0
    { '.', '-', '-', '-', '-',   0 }, // 1
    { '.', '.', '-', '-', '-',   0 }, // 2
    { '.', '.', '.', '-', '-',   0 }, // 3
    { '.', '.', '.', '.', '-',   0 }, // 4
    { '.', '.', '.', '.', '.',   0 }, // 5
    { '-', '.', '.', '.', '.',   0 }, // 6
    { '-', '-', '.', '.', '.',   0 }, // 7
    { '-', '-', '-', '.', '.',   0 }, // 8
    { '-', '-', '-', '-', '.',   0 }, // 9
    { ' ',   0,   0,   0,   0,   0 }  // espaço
};

typedef struct
{
    int pos;
    char *color;
    int period_ms;
} st_led_param_t;

st_led_param_t green = {
    6,
    GREEN,
    50};

TaskHandle_t ledTask_hdlr, morseTask_hdlr;
QueueHandle_t keyQueue = NULL, charQueue = NULL, morseQueue = NULL;

#include <termios.h>

int stop = 0;
static void prvTask_getChar(void *pvParameters)
{
    char key;
    int n;

    /* I need to change  the keyboard behavior to
    enable nonblock getchar */
    struct termios initial_settings,
        new_settings;

    tcgetattr(0, &initial_settings);

    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 1;

    tcsetattr(0, TCSANOW, &new_settings);
    /* End of keyboard configuration */
    while(!stop)
    {
        key = getchar();
        if (key > 0)
        {
            /* Adiciona o key precionada no keyQueue para ser tratada posteriormente */
            xQueueSend(keyQueue, &key, 0);
        }
    }


    tcsetattr(0, TCSANOW, &initial_settings);
    ENABLE_CURSOR();
    exit(0);
    vTaskDelete(NULL);
}

static void prvTask_processingData(void *pvParameters)
{
    char key;
    while(1)
    {
        if (xQueueReceive(keyQueue, &key, portMAX_DELAY) == pdPASS){

            /* Trata o botão precionad */
            switch(key){
                case KEY_ESC: /* Finaliza o programa */
                    stop = 1;
                    break;
                case KEY_ENTER: /* Executa a task para codificar os dados em morse */
                    vTaskResume(morseTask_hdlr);
                    break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                case 'J':
                case 'K':
                case 'L':
                case 'M':
                case 'N':
                case 'O':
                case 'P':
                case 'Q':
                case 'R':
                case 'S':
                case 'T':
                case 'U':
                case 'W':
                case 'X':
                case 'Y':
                case 'Z':
                case ' ':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                case 'i':
                case 'j':
                case 'k':
                case 'l':
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    /* Adiciona o charactere para se decodificado */
                    xQueueSend(charQueue, &key, 0);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

static void prvTask_morseCodification(void *pvParameters)
{

    char charactere;
    char indexCode, index;
    while(1)
    {
        vTaskSuspend(morseTask_hdlr); /* Suspende a task até que a task processingData requisite que um novo codigo seja decodificado */

        while(xQueueReceive(charQueue, &charactere, 0) == pdPASS){

            if(charactere >= 'a' && charactere <= 'z'){
                index = charactere - 'a';
            } else if(charactere >= 'A' && charactere <= 'Z'){
                index = charactere - 'A';
            } else if(charactere >= '0' && charactere <= '9'){
                index = charactere - '0' + 25;
            } else {
                index = 35;
            }
            // gotoxy(0, 5);
            // printf("%c - %u", charactere, index);

            indexCode = 0;
            while(morseCode[index][indexCode]){
                xQueueSend(morseQueue, &morseCode[index][indexCode], 0);

                indexCode++;
            }
        }
    }
}

static void prvTask_led(void *pvParameters)
{
    uint8_t status_led = 0;
    char charactere;
    int delay;

    while(1)
    {
        if(xQueueReceive(morseQueue, &charactere, 0) == pdPASS){
            // gotoxy(0, 4);
            // printf("%c", charactere);

            if(charactere == ' '){
                gotoxy(5, 2);
                printf("%s ", BLACK);
                fflush(stdout);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            } else {
                if(charactere == '.'){
                    delay = 150;
                } else {
                    delay = 400;
                }

                gotoxy(5, 2);
                printf("%s⬤", GREEN);
                fflush(stdout);
                vTaskDelay(delay / portTICK_PERIOD_MS);

                gotoxy(5, 2);
                printf("%s ", BLACK);
                fflush(stdout);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
        }

    }

    vTaskDelete(NULL);
}

void app_run(void)
{
    keyQueue = xQueueCreate(100, // Queue length
                               1); // Queue item size
    charQueue = xQueueCreate(100, // Queue length
                               1); // Queue item size
    morseQueue = xQueueCreate(100, // Queue length
                               1); // Queue item size

    if (keyQueue == NULL)
    {
        printf("Fail on create queue\n");
        exit(1);
    }

    clear();
    DISABLE_CURSOR();
    printf(
        "╔════════╗\n"
        "║        ║\n"
        "╚════════╝\n");

    xTaskCreate(prvTask_getChar, "Get_key", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, NULL); // Leitura do teclado
    xTaskCreate(prvTask_processingData, "Processing_key", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, NULL); // Codificacao frase
    xTaskCreate(prvTask_morseCodification, "Morse", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, &morseTask_hdlr); // Codificacao Morse
    xTaskCreate(prvTask_led, "Led", configMINIMAL_STACK_SIZE, NULL, TASK4_PRIORITY, &ledTask_hdlr); // Apresentacao Morse

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks      to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for (;;)
    {
    }
}