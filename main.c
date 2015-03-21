#include <pthread.h>
#include <md5.h>
#include <comm.h>
#include <time.h>

#define CORES 4

pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done = PTHREAD_COND_INITIALIZER;

pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready = PTHREAD_COND_INITIALIZER;

pthread_t workers[CORES];

// When a thread finds a result, signal all other threads to stop.
volatile uint8_t stop = 0;

// The actual solution.
volatile uint32_t solution;

// Message.
uint32_t msg[15];
// Difficulty.
uint8_t diff;

void *worker(void *); 

int main(int argc, char * argv[]) {
	uint8_t i;
	uint8_t * res;
	comm_init();
	pthread_create(&workers[0], NULL, worker, (void *)0x00000000);
	pthread_create(&workers[1], NULL, worker, (void *)0x08000000);
	pthread_create(&workers[2], NULL, worker, (void *)0x80000000);
	pthread_create(&workers[3], NULL, worker, (void *)0x88000000);
	while (1) {
		res = get_block();
		diff =  *res++;
		//printf("Difficulty: %d\n", diff);
		for (i = 0; i < 15; i++) {
			msg[i] = *res++;
			msg[i] |= *res++ << 8;
			msg[i] |= *res++ << 16;
			msg[i] |= *res++ << 24;
		}
		stop = 0;
		pthread_cond_broadcast(&ready);
		pthread_mutex_lock(&done_mutex);
		pthread_cond_wait(&done, &done_mutex);
		pthread_mutex_unlock(&done_mutex);

		//printf("%x\n", solution);
		submit_block(solution);
		//if (submit_block(solution)) putchar('.');
		//else putchar('x');
		fflush(0);
	}
	return 0;
}

void *worker(void * p) {
	uint32_t start = (uint32_t)p;
	MD5_ctx * c = malloc(sizeof(MD5_ctx));
	printf("Starting from %x\n", start);
	fflush(0);
	while (1) {
		// Wait for msg + diff to become avaliable.
		pthread_mutex_lock(&ready_mutex);
		pthread_cond_wait(&ready, &ready_mutex);
		pthread_mutex_unlock(&ready_mutex);
		// Prepare the md5 context.
		md5_setup(c, start, diff, msg);
		// Do hashing. Hard.
		while ((md5_hash(c) < diff) && !stop)  {
			c->msg[0]++;
		}
		if (!stop) {
			pthread_mutex_lock(&done_mutex);
			stop = 0x01;
			solution = ENDSWAP(c->msg[0]);
			pthread_cond_signal(&done);
			pthread_mutex_unlock(&done_mutex);
		}
	}
}
