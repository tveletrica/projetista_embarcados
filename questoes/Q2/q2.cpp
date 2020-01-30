#include <iostream>
#include <vector>
#include <thread>

static const int THREAD_COUNT = 10;

static void print_log(int id)
{
    std::cout << "--------------------" <<'\n';
    std::cout << "Iniciando bloco " << id << '\n';
    std::cout << "Hello world from thread " << id << '\n';
    std::cout << "Fim do bloco " << id << '\n';
    std::cout << "--------------------" << '\n';
}


int main()
{
    std::vector<std::thread> v;

    for (size_t i = 0; i < THREAD_COUNT; i++)
    {
        v.emplace_back(print_log, i);
    }

    for (auto &t : v)
    {
        t.join();
    }

    return 0;
}
