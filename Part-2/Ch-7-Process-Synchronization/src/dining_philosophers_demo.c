#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define NUM_PHILOSOPHERS 5
#define EATING_TIME 2
#define THINKING_TIME 3
#define SIMULATION_TIME 30

typedef enum {
    THINKING,
    HUNGRY,
    EATING
} philosopher_state_t;

typedef struct {
    philosopher_state_t state[NUM_PHILOSOPHERS];
    pthread_cond_t can_eat[NUM_PHILOSOPHERS];
    pthread_mutex_t monitor_lock;
    int meals_eaten[NUM_PHILOSOPHERS];
} monitor_table_t;

typedef struct {
    sem_t *forks[NUM_PHILOSOPHERS];
    sem_t *room;
} semaphore_table_t;

monitor_table_t monitor_table;
semaphore_table_t semaphore_table;
volatile int simulation_running = 1;

void monitor_init() {
    pthread_mutex_init(&monitor_table.monitor_lock, NULL);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        monitor_table.state[i] = THINKING;
        pthread_cond_init(&monitor_table.can_eat[i], NULL);
        monitor_table.meals_eaten[i] = 0;
    }
}

void semaphore_init() {
    char sem_name[32];
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        snprintf(sem_name, sizeof(sem_name), "/fork_%d", i);
        sem_unlink(sem_name);
        semaphore_table.forks[i] = sem_open(sem_name, O_CREAT, 0644, 1);
        if (semaphore_table.forks[i] == SEM_FAILED) {
            perror("sem_open failed for fork");
            exit(1);
        }
    }
    
    sem_unlink("/room");
    semaphore_table.room = sem_open("/room", O_CREAT, 0644, NUM_PHILOSOPHERS - 1);
    if (semaphore_table.room == SEM_FAILED) {
        perror("sem_open failed for room");
        exit(1);
    }
}

void test_philosopher(int id) {
    int left = (id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    if (monitor_table.state[id] == HUNGRY &&
        monitor_table.state[left] != EATING &&
        monitor_table.state[right] != EATING) {
        
        monitor_table.state[id] = EATING;
        pthread_cond_signal(&monitor_table.can_eat[id]);
    }
}

void pickup_forks_monitor(int id) {
    pthread_mutex_lock(&monitor_table.monitor_lock);
    
    monitor_table.state[id] = HUNGRY;
    printf("Philosopher %d is hungry\n", id);
    
    test_philosopher(id);
    
    while (monitor_table.state[id] != EATING) {
        pthread_cond_wait(&monitor_table.can_eat[id], &monitor_table.monitor_lock);
    }
    
    printf("Philosopher %d picked up forks and is eating\n", id);
    pthread_mutex_unlock(&monitor_table.monitor_lock);
}

void putdown_forks_monitor(int id) {
    pthread_mutex_lock(&monitor_table.monitor_lock);
    
    monitor_table.state[id] = THINKING;
    monitor_table.meals_eaten[id]++;
    printf("Philosopher %d put down forks (meal #%d)\n", id, monitor_table.meals_eaten[id]);
    
    int left = (id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    test_philosopher(left);
    test_philosopher(right);
    
    pthread_mutex_unlock(&monitor_table.monitor_lock);
}

void pickup_forks_asymmetric(int id) {
    if (id == NUM_PHILOSOPHERS - 1) {
        sem_wait(semaphore_table.forks[(id + 1) % NUM_PHILOSOPHERS]);
        sem_wait(semaphore_table.forks[id]);
    } else {
        sem_wait(semaphore_table.forks[id]);
        sem_wait(semaphore_table.forks[(id + 1) % NUM_PHILOSOPHERS]);
    }
    printf("Philosopher %d picked up forks (asymmetric)\n", id);
}

void putdown_forks_asymmetric(int id) {
    sem_post(semaphore_table.forks[(id + 1) % NUM_PHILOSOPHERS]);
    sem_post(semaphore_table.forks[id]);
    printf("Philosopher %d put down forks (asymmetric)\n", id);
}

void pickup_forks_tanenbaum(int id) {
    sem_wait(semaphore_table.room);
    sem_wait(semaphore_table.forks[id]);
    sem_wait(semaphore_table.forks[(id + 1) % NUM_PHILOSOPHERS]);
    printf("Philosopher %d picked up forks (Tanenbaum)\n", id);
}

void putdown_forks_tanenbaum(int id) {
    sem_post(semaphore_table.forks[(id + 1) % NUM_PHILOSOPHERS]);
    sem_post(semaphore_table.forks[id]);
    sem_post(semaphore_table.room);
    printf("Philosopher %d put down forks (Tanenbaum)\n", id);
}

void* philosopher_monitor(void* arg) {
    int id = *(int*)arg;
    
    while (simulation_running) {
        printf("Philosopher %d is thinking\n", id);
        sleep(THINKING_TIME);
        
        if (!simulation_running) break;
        
        pickup_forks_monitor(id);
        sleep(EATING_TIME);
        putdown_forks_monitor(id);
    }
    
    return NULL;
}

void* philosopher_asymmetric(void* arg) {
    int id = *(int*)arg;
    int meals = 0;
    
    while (simulation_running) {
        printf("Philosopher %d is thinking\n", id);
        sleep(THINKING_TIME);
        
        if (!simulation_running) break;
        
        pickup_forks_asymmetric(id);
        printf("Philosopher %d is eating (meal #%d)\n", id, ++meals);
        sleep(EATING_TIME);
        putdown_forks_asymmetric(id);
    }
    
    return NULL;
}

void* philosopher_tanenbaum(void* arg) {
    int id = *(int*)arg;
    int meals = 0;
    
    while (simulation_running) {
        printf("Philosopher %d is thinking\n", id);
        sleep(THINKING_TIME);
        
        if (!simulation_running) break;
        
        pickup_forks_tanenbaum(id);
        printf("Philosopher %d is eating (meal #%d)\n", id, ++meals);
        sleep(EATING_TIME);
        putdown_forks_tanenbaum(id);
    }
    
    return NULL;
}

void run_simulation(void* (*philosopher_func)(void*), const char* strategy_name) {
    printf("\n=== Running %s Strategy ===\n", strategy_name);
    
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int philosopher_ids[NUM_PHILOSOPHERS];
    
    simulation_running = 1;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher_func, &philosopher_ids[i]);
    }
    
    sleep(SIMULATION_TIME);
    simulation_running = 0;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    if (philosopher_func == philosopher_monitor) {
        printf("\nMeals eaten per philosopher:\n");
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            printf("Philosopher %d: %d meals\n", i, monitor_table.meals_eaten[i]);
        }
    }
}

int main() {
    printf("Dining Philosophers Problem Demonstration\n");
    printf("Simulation will run for %d seconds\n", SIMULATION_TIME);
    
    monitor_init();
    semaphore_init();
    
    run_simulation(philosopher_monitor, "Monitor");
    run_simulation(philosopher_asymmetric, "Asymmetric");
    run_simulation(philosopher_tanenbaum, "Tanenbaum");
    
    // Cleanup
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&monitor_table.can_eat[i]);
        sem_close(semaphore_table.forks[i]);
        
        char sem_name[32];
        snprintf(sem_name, sizeof(sem_name), "/fork_%d", i);
        sem_unlink(sem_name);
    }
    
    pthread_mutex_destroy(&monitor_table.monitor_lock);
    sem_close(semaphore_table.room);
    sem_unlink("/room");
    
    return 0;
} 