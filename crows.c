#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#define new(name, type, n) type *name = (type*)malloc((n)*sizeof(type))
#define iter(i, to) for(int i = 0; i < (to); i++)
#define normal(n) n = n < 1 ? 10 : n > 20 ? 20 : n
#define critical(mutex, cs) sem_wait(&mutex); cs sem_post(&mutex);

int m, n, t;
int empty_pot, hungry_crows, next_pot;
int mother_gone, dont_wait;

sem_t awake, full_pot, hungry_crows_mutex, next_pot_mutex;



void goto_sleep() {
    printf("mother is going to sleep\n");
    sem_wait(&awake);
}
void food_ready() {
    empty_pot = 0;
    printf("food is ready. %d feeding left\n", t - 1);
    iter(i, m) {
        sem_post(&full_pot); // give premission to eat
    }
}

void* mother_p(void*) {
    printf("mother is here\n");
    do {
        goto_sleep(); // take a nap
        sleep(1); // prepare food
        food_ready(); // make food available
    } while(--t);
    mother_gone = 1;
    return NULL;
}


void ready_to_eat(int id) {
    critical(hungry_crows_mutex,
        hungry_crows ++;
        iter(i, id + 1) printf("  "); printf("baby %d is hungry\n", id);
        if(empty_pot == m && hungry_crows == 1) {
            // when I am hungry, if all pots are empty and no one is hungry
            // I will wake mom.
            // but if someone else is hungry too,
            // either she must have called mom.
            // or her brother called mom when he ate from the last pot. so I won't !
            iter(i, id + 1) printf("  "); printf("baby %d is waking mom. there is no food\n", id);
            sem_post(&awake);
        }
    );

    sem_wait(&full_pot); // wait for mothers premission to eat
    if(dont_wait) return;
    critical(next_pot_mutex,
        iter(i, id + 1) printf("  "); printf("baby %d is eating from pot %d\n", id, next_pot);
        next_pot = (next_pot + 1) % m;
    );
}
void finish_eating(int id) {
    iter(i, id + 1) printf("  "); printf("baby %d ate\n", id);
    critical(hungry_crows_mutex,
        hungry_crows --;
        empty_pot ++;
        if(empty_pot == m && hungry_crows > 0) {
            // when I ate, if all pots are empty and my sister is hungry
            // I will wake mom for her
            if(mother_gone == 0) {
                iter(i, id + 1) printf("  "); printf("baby %d is waking mom. someone still hungry\n", id);
                sem_post(&awake);
            } else {
                // mother is gone and someone is still waiting for food !
                // I should tell them to go.
                iter(i, id + 1) printf("  "); printf("baby %d is telling others to go\n", id);
                dont_wait = 1;
                iter(i, hungry_crows) {
                    sem_post(&full_pot);
                }
            }
        }
    );
}

void* baby_p(void* arg) {
    int id = *((int*)arg);
    iter(i, id + 1) printf("  "); printf("baby %d is here\n", id);
    while(1) {
        iter(i, id + 1) printf("  "); printf("baby %d is playing\n", id);
        sleep(2); // play for a while
        ready_to_eat(id); // get hungry
        if(dont_wait) {
            return NULL;
        }
        sleep(1); // eat for a while
        finish_eating(id); // finish eating
        if(t <= 0) {
            // if there is no food I must make my own life !
            return NULL;
        }
    }
}

int main() {
    // input
    printf("m n t >");
    scanf("%d%d%d", &m, &n, &t);
    normal(m);
    normal(n);
    normal(t);

    // init
    sem_init(&awake, 0, 0);
    sem_init(&full_pot, 0, 0);
    sem_init(&hungry_crows_mutex, 0, 1);
    sem_init(&next_pot_mutex, 0, 1);
    empty_pot = m;
    hungry_crows = 0;
    next_pot = 0;
    mother_gone = dont_wait = 0;

    // make threads
    pthread_t mother;
    pthread_create(&mother, NULL, mother_p, NULL);
    new(baby, pthread_t, n);
    new(baby_id, int, n);
    iter(i, n) {
        baby_id[i] = i;
        pthread_create(&baby[i], NULL, baby_p, baby_id + i);
    }
    // wait for threads to join
    pthread_join(mother, NULL);
    printf("mother is gone !\n");
    iter(i, n) {
        pthread_join(baby[i], NULL);
        iter(j, n + 1) printf("  "); printf("baby %d is gone !\n", i);
    }

    free(baby);
    free(baby_id);
    return 0;
}