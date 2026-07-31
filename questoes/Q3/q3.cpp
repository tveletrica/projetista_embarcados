#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

// ---------------------------------------------------------------------------
// Fila thread-safe com mutex + condition_variable.
// O consumidor bloqueia aguardando novos itens e o flag done indica o fim
// da producao, permitindo encerrar a thread de forma ordenada.
// ---------------------------------------------------------------------------
class ThreadSafeQueue
{
public:
    void push(const std::string &item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(item);
        }
        m_cv.notify_one();
    }

    // Retorna o proximo item, ou std::nullopt quando a fila estiver vazia
    // e a producao ja foi concluida.
    std::optional<std::string> pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_queue.empty() || m_done; });

        if (m_queue.empty())
            return std::nullopt;

        std::string item = m_queue.front();
        m_queue.pop();
        return item;
    }

    void finish()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done = true;
        }
        m_cv.notify_all();
    }

private:
    std::queue<std::string> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_done = false;
};

// ---------------------------------------------------------------------------
// Extrai o conteudo das tags <payload></payload> da linha e retorna todas as
// ocorrencias. Como o XML pode conter varias ocorrencias, todas saem.
// ---------------------------------------------------------------------------
static std::vector<std::string> extract_payloads(const std::string &line)
{
    const std::string open_tag = "<payload>";
    const std::string close_tag = "</payload>";

    std::vector<std::string> payloads;
    size_t pos = 0;
    while (true)
    {
        size_t start = line.find(open_tag, pos);
        if (start == std::string::npos)
            break;

        start += open_tag.size();
        size_t end = line.find(close_tag, start);
        if (end == std::string::npos)
            break;

        payloads.push_back(line.substr(start, end - start));
        pos = end + close_tag.size();
    }
    return payloads;
}

// Thread produtora: le o arquivo linha a linha e envia para a fila.
static void producer(ThreadSafeQueue &queue, const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Erro: nao foi possivel abrir o arquivo " << path << '\n';
        queue.finish();
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        queue.push(line);
    }
    file.close();
    queue.finish();
}

// Thread consumidora: le a fila e imprime apenas o conteudo do payload.
static void consumer(ThreadSafeQueue &queue)
{
    while (auto item = queue.pop())
    {
        for (const auto &payload : extract_payloads(*item))
            std::cout << payload << '\n';
    }
}

int main(int argc, char *argv[])
{
    const std::string input_path = (argc > 1) ? argv[1] : "input.xml";

    ThreadSafeQueue queue;

    std::thread producer_thread(producer, std::ref(queue), input_path);
    std::thread consumer_thread(consumer, std::ref(queue));

    producer_thread.join();
    consumer_thread.join();

    return 0;
}
