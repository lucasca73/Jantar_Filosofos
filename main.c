#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/types.h"
#include "pthread.h"
#include "semaphore.h"


// Cores para ajudar a visualizar estados
#define blue    "\x1B[34m"
#define yellow  "\x1B[33m"
#define green   "\x1B[32m"
#define red     "\x1B[31m"
#define reset   "\x1B[0m"

#define clear() printf("\033[H\033[J") // Limpa tela

#define N           5           // número de filósofos
#define LEFT        (i+N-1)%N   // número do vizinho à esquerda de i
#define RIGHT       (i+1)%N     // número do vizinho à direito de i
#define THINKING    0           // o filósofo está pensando
#define HUNGRY      1           // o filósofo está tentando pegar garfos
#define EATING      2           // o filósofo está comendo
#define TRUE        1  

struct phil_state{
    int value;
    int eat_times;  
};

typedef struct phil_state phil_state;

phil_state* state[N];           // arranjo para controlar o estado de cada um
sem_t mutex;                    // exclusão mútua para as regiões críticas
sem_t s[N];                     // um semáforo por filósofo

// protótipos */
void* philosopher(void* arg);
void take_forks(int i);
void put_forks(int i);
int propose_eat(int i);
void eat(int i);
void think(int i);
void show_philosophers();


int main() {
    int i = 0;

    sem_init(&mutex, TRUE, 1);

    for(i = 0; i < N; i++) {
        sem_init(&s[i], TRUE, 1);
    }

    pthread_t tphil[N];

   // criando os 5 filósofos em thread
    for(i = 0; i < N; i++) {
        state[i] = (phil_state *) malloc(sizeof(phil_state));
        state[i]->value = malloc(sizeof(int));
        state[i]->eat_times = malloc(sizeof(int));
        state[i]->eat_times = 0;

        pthread_create(&tphil[i], NULL, (void *) philosopher, (void*) i);
    }

    while(1){
        show_philosophers();
        usleep(500);
    }

    return 0;
}

void* philosopher(void* arg) { //i: o número do filósofo, de 0 a N-1
    int i = arg;
    //printf("rodanddo filosofo: %d\n", i);

    while(TRUE) {               // repete para sempre
        think(i);               // o filósofo está pensando
        take_forks(i);          // pega dois garfos ou bloqueia
        eat(i);                 // o filósofo está comendo
        put_forks(i);           // devolve os dois garfos à mesa
    }

    pthread_exit(NULL);
}

void take_forks(int i) {        

    int can_eat = 0;
    
    sem_wait(&mutex);                     // entra na região crítica
    can_eat = propose_eat(i);             // tenta pegar dois garfos
    if (can_eat == 0)  {
        //printf("%s filosofo %d: ESTOU COM FOME!\n", red, i);
        sem_wait(&s[i]);            // bloqueia o filosofo caso nao pegue os garfos
    }
               
    sem_post(&mutex);           // sai da região crítica
}

void put_forks(int i) {    
    sem_wait(&mutex);           // entra na região crítica
    if (state[i]->value == EATING){
        state[i]->value = THINKING;  // o filósofo acabou de comer
        propose_eat(LEFT);          // vê se o vizinho da esquerda pode comer agora
        propose_eat(RIGHT);         // vê se o vizinho da direito pode comer agora
    }

    sem_post(&mutex);           // sai da região crítica
}

int propose_eat(int i) {

    int vizinhos_nao_estao_comendo = state[LEFT]->value != EATING && state[RIGHT]->value != EATING;
    int estou_com_fome = state[i]->value == HUNGRY;

    if ( estou_com_fome && vizinhos_nao_estao_comendo ) {
        state[i]->value = EATING;
        state[i]->eat_times += 1;
        //printf("%s filosofo %d: pegando garfos\n",yellow, i);
        sem_post(&s[i]);
        return 1;
    }else{
        return 0;
    }
}

void eat(int i) {
    //printf("%s Filosofo %d: comendo...\n", yellow, i);
    sleep( (rand()%4) + 2);  // comer demora um tempo aleatório
    //printf("%s Filosofo %d: terminei.\n", green, i);
}

void think(int i) {

    if (state[i]->value != HUNGRY) {
        //printf("%s Filosofo %d: pensando!\n", blue,i);
        sleep( rand()%10 ); // comer demora um tempo aleatório
        state[i]->value = HUNGRY; // depois de pensar da fome
    }
}

void show_philosophers(){

    clear();
    int total_eated = 0;
    int ph_eating = 0;
    int ph_hungry = 0;
    int ph_thinking = 0;
    int st = 0;
    int eat = 0;
    char* st_name = "HUNGRYINGDYING";
    char* color;

    for (int k = 0; k < N; k++){
        st = state[k]->value;
        eat = state[k]->eat_times;
        total_eated += eat;
        
        switch(st){

            case HUNGRY:
                color = red;
                st_name = "Hungry";
                ph_hungry += 1;
            break;

            case EATING:

                color = green;
                st_name = "Eating";
                ph_eating += 1;
            break;

            case THINKING:
                color = blue;
                st_name = "Thinking";
                ph_thinking += 1;
            break;

            default:
                color = yellow;
                st_name = "initing ...";
            break;
        }

        printf("%s filosofo%d\t- %s \t eated: %d\n", color, k, st_name, eat);
    }

    printf("%sfilosofos %s%d \t",reset, yellow, N);
    printf("%stotal comido: %s%d \t", reset, green, total_eated);
    printf("%spensando: %s%d \t", reset, blue, ph_thinking);
    printf("%scomendo: %s%d \t", reset, green, ph_eating);
    printf("%scom fome: %s%d \n", reset, red, ph_hungry);
}