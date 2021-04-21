#include <stdint.h>
#include <cstdlib>
#include <pthread.h>
#include <iostream>

using namespace std;

#define CAS(_p, _u, _v) (__atomic_compare_exchange_n (_p, _u, _v, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE))

class test_lock
{
public:
	int sema = 0;
	bool suspend(void){
		int64_t val;
		do{
			val = sema;
			if(val < 0)
			return false;
		}while(!CAS(&sema, &val, -1));

		int64_t wait = 0 - val - 1;
		while(val && sema != wait){
			asm("nop");
		}
		return true;
    }

    bool lock(void){
		int64_t val = sema;
		while(val > -1){
			if(CAS(&sema, &val, val+1))
			return true;
			val = sema;
		}
		return false;
    }

    void unlock(void){
		int64_t val = sema;
		while(!CAS(&sema, &val, val-1)){
			val = sema;
		}
    }

} tlock;

void* thread_lock(void* arg) {
	if(tlock.lock())
		std::cout << "lock successfully\n" << std::endl;
	else 
		std::cout << "lock failed\n" << std::endl;
	return NULL;
}

void* thread_unlock(void* arg) {
	getchar();
	tlock.unlock();
	std::cout << "unlock successfully\n"
			  << std::endl;
	return NULL;
}

void* thread_suspend(void* arg) {
	if(tlock.suspend())
		std::cout << "suspend successfully\n" << std::endl;
	else 
		std::cout << "suspend failed\n" << std::endl;
	return NULL;
}

int main() {
	pthread_t p[4];
	pthread_create(&p[0], NULL, thread_lock, NULL);
	pthread_create(&p[1], NULL, thread_suspend, NULL);
	pthread_create(&p[2], NULL, thread_unlock, NULL);
	pthread_create(&p[3], NULL, thread_suspend, NULL);

	for (int i = 0; i < 4; i++) {
		pthread_join(p[i], NULL);
	}
		return 0;
}
