#include <iostream>
#include <unistd.h>
/*
int main() {
    int valor = 0;

    while (valor <= 100) {
        std::cout << "\rVALOR : " << valor << std::flush;
        valor++;
        usleep(1000000);  // Esperar 1 segundo (en microsegundos)
    }

    std::cout << std::endl;
    return 0;
}

*/

#include <thread>
#include <mutex>
std::mutex mtx;


void printValue(int id, int delay) {
    int valor = 0;
    while (valor <= 100) {
std::unique_lock<std::mutex> lock(mtx);
        std::cout << "\rVALOR " << id << " : " << valor << std::flush;
lock.unlock();
        valor++;
        usleep(delay);
    }
//std::cout << "\rVALOR " << id << " : 100" << std::endl;
//    std::cout << std::endl;
}

int main() {
    	std::thread thread1(printValue, 1, 1000000);  // 1 segundo
 	std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    	std::thread thread2(printValue, 2, 2000000);  // 2 segundos

    thread1.join();
    thread2.join();

std::cout << std::endl;
    return 0;
}
