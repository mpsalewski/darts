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
