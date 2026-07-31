#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <syslog.h>

static const int THREAD_COUNT = 10;

// Mutex para serializar a escrita no syslog, evitando que as threads
// interrompam as mensagens umas das outras (condicao de corrida).
static std::mutex g_log_mutex;

// Monta a mensagem completa em uma unica string, do inicio ao fim.
static std::string build_message(int id)
{
    std::ostringstream oss;
    oss << "--------------------\n";
    oss << "Iniciando bloco " << id << '\n';
    oss << "Hello world from thread " << id << '\n';
    oss << "Fim do bloco " << id << '\n';
    oss << "--------------------\n";
    return oss.str();
}

// Envia a mensagem ao syslog em uma unica chamada, garantindo que saia do
// inicio ao fim sem interrupcoes pelas outras threads (mutex).
static void print_log(int id)
{
    std::lock_guard<std::mutex> lock(g_log_mutex);
    syslog(LOG_INFO, "%s", build_message(id).c_str());
}

int main()
{
    openlog("q2", LOG_PID | LOG_CONS, LOG_USER);

    std::vector<std::thread> v;

    for (size_t i = 0; i < THREAD_COUNT; i++)
    {
        v.emplace_back(print_log, i);
    }

    for (auto &t : v)
    {
        t.join();
    }

    closelog();
    return 0;
}
