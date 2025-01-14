/***************** C ************************/

#include <stdio.h>
#include <threads.h>

typedef struct {
    mtx_t mutex;     // Mutex zur Synchronisation
    int sharedData;  // Zu schreibende/lesende Variable
    int ready;       // Statusvariable
} SharedResource;

int producer(void* arg) {
    SharedResource* resource = (SharedResource*)arg;

    mtx_lock(&resource->mutex); // Zugriff sperren
    resource->sharedData = 42;  // Daten schreiben
    resource->ready = 1;        // Status setzen
    printf("Producer hat die Daten geschrieben.\n");
    mtx_unlock(&resource->mutex); // Zugriff freigeben

    return 0;
}

int consumer(void* arg) {
    SharedResource* resource = (SharedResource*)arg;

    mtx_lock(&resource->mutex); // Zugriff sperren
    while (!resource->ready) {  // Warten, bis Daten bereit sind
        mtx_unlock(&resource->mutex); // Mutex freigeben, um andere Threads nicht zu blockieren
        thrd_yield();                 // Zeit für andere Threads geben
        mtx_lock(&resource->mutex);   // Mutex erneut sperren
    }
    printf("Consumer liest: %d\n", resource->sharedData);
    mtx_unlock(&resource->mutex); // Zugriff freigeben

    return 0;
}

int main() {
    thrd_t producerThread, consumerThread;
    SharedResource resource;

    // Initialisiere die gemeinsame Ressource
    mtx_init(&resource.mutex, mtx_plain);
    resource.sharedData = 0;
    resource.ready = 0;

    // Erstelle Producer- und Consumer-Threads
    thrd_create(&producerThread, producer, &resource);
    thrd_create(&consumerThread, consumer, &resource);

    // Warte auf beide Threads
    thrd_join(producerThread, NULL);
    thrd_join(consumerThread, NULL);

    // Zerstöre den Mutex
    mtx_destroy(&resource.mutex);

    return 0;
}


/************* c++ **************/
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

struct SharedResource {
    std::mutex mutex;                  // Mutex zur Synchronisation
    std::condition_variable cond_var;  // Bedingungsvariable
    int sharedData = 0;                // Zu schreibende/lesende Variable
    bool ready = false;                // Statusvariable
};

void producer(SharedResource& resource) {
    std::unique_lock<std::mutex> lock(resource.mutex); // Mutex sperren
    resource.sharedData = 42;                         // Daten schreiben
    resource.ready = true;                            // Status setzen
    std::cout << "Producer hat die Daten geschrieben.\n";
    resource.cond_var.notify_one();                   // Signal an Consumer senden
}

void consumer(SharedResource& resource) {
    std::unique_lock<std::mutex> lock(resource.mutex); // Mutex sperren
    resource.cond_var.wait(lock, [&resource] { return resource.ready; }); // Auf Signal warten
    std::cout << "Consumer liest: " << resource.sharedData << "\n";
    // Mutex wird automatisch freigegeben, wenn `lock` aus dem Gültigkeitsbereich geht
}

int main() {
    SharedResource resource;

    // Erstelle Producer- und Consumer-Threads
    std::thread producerThread(producer, std::ref(resource));
    std::thread consumerThread(consumer, std::ref(resource));

    // Warte auf beide Threads
    producerThread.join();
    consumerThread.join();

    return 0;
}
