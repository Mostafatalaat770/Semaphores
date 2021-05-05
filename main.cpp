#include <iostream>
#include <semaphore.h>
#include <ctime>
#include <unistd.h>
#include <queue>
#include <thread>
using namespace std;
int counter;
int bufferSize;
int numOfThreads;
sem_t wsem, mutex, notempty, notfull;
int t;
queue<int> buffer;

int randomNumber()
{
    srand(clock());
    return rand() % 1000000 + 100;
}

void reader()
{
    while (true)
    {
        cout << "Monitor thread: waiting to read counter." << endl;
        sem_wait(&notfull);
        sem_wait(&mutex);
        sem_wait(&wsem);
        cout << "Monitor thread: reading a counter of value = " << counter << endl;
        if (buffer.size() < bufferSize)
        {
            buffer.push(counter);
            cout << "Monitor thread: inserted value in buffer = " << buffer.back() << endl;
            cout << "Monitor thread: inserted value in position = " << buffer.size() - 1 << endl;
        }
        else
        {
            cout << "Monitor thread: buffer is full!" << endl;
        }
        counter = 0;
        sem_post(&notempty);
        sem_post(&mutex);
        sem_post(&wsem);
        usleep(t);
    }
}

void writer(int threadId)
{
    cout << "Thread " << threadId << ": recieved a message." << endl;
    while (true)
    {
        cout << "Thread " << threadId << ": waiting to write." << endl;
        sem_wait(&wsem);
        counter++;
        cout << "Thread " << threadId << ": now adding to counter, counter value = " << counter << endl;
        sem_post(&wsem);

        usleep(randomNumber());
    }
}

void consumer()
{
    int pos = 0;
    while (true)
    {
        if (buffer.size() == 0)
            cout << "Collector thread: nothing is in the bufer!" << endl;

        sem_wait(&notempty);
        sem_wait(&mutex);

        cout << "Collector thread: collected value = " << buffer.front() << endl;
        cout << "Collector thread: collected value from position = " << pos++ % bufferSize << endl;
        buffer.pop();

        sem_post(&notfull);
        sem_post(&mutex);

        usleep(randomNumber());
    }
}

int main()
{
    cout << "Insert the number of threads: " << endl;
    cin >> numOfThreads;
    cout << "Insert the size of buffer: " << endl;
    cin >> bufferSize;

    sem_init(&wsem, 0, 1);
    sem_init(&notfull, 0, bufferSize);
    sem_init(&notempty, 0, 0);
    sem_init(&mutex, 0, 1);

    cout << "Insert size of t for monitor: " << endl;
    cin >> t;

    std::thread writers[numOfThreads];
    std::thread monitor = thread(reader);
    std::thread collector = thread(consumer);

    for (int i = 0; i < numOfThreads; i++)
    {
        writers[i] = thread(writer, i);
    }
    monitor.join();
    collector.join();
    for (int i = 0; i < numOfThreads; i++)
    {
        writers[i].join();
    }
    return 0;
}
