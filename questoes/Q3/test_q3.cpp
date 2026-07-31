// ---------------------------------------------------------------------------
// Testes unitarios do Q3 (fila thread-safe e extracao de payload).
//
// q3.cpp e incluido diretamente com o main renomeado via macro, permitindo
// acessar as classes/funcoes estaticas dentro do mesmo arquivo de teste.
// ---------------------------------------------------------------------------

#define main q3_original_main
#include "q3.cpp"
#undef main

#include <atomic>
#include <cstdio>

static int g_failures = 0;

static void check(bool cond, const char *what)
{
    if (cond)
    {
        printf("[OK] %s\n", what);
    }
    else
    {
        printf("[FALHOU] %s\n", what);
        ++g_failures;
    }
}

// ---------------------------------------------------------------------------
// ThreadSafeQueue
// ---------------------------------------------------------------------------

static void test_queue_fifo_order()
{
    ThreadSafeQueue q;
    for (int i = 1; i <= 5; ++i)
        q.push("item" + std::to_string(i));

    q.finish();

    bool ok = true;
    for (int i = 1; i <= 5; ++i)
    {
        auto item = q.pop();
        ok = ok && item.has_value() && *item == "item" + std::to_string(i);
    }
    check(ok, "fila preserva a ordem FIFO");

    auto empty = q.pop();
    check(!empty.has_value(), "pop apos o fim retorna vazio");
}

static void test_queue_pop_blocks_until_done()
{
    ThreadSafeQueue q;

    // Consumidor deve retornar nullopt quando finish() for chamado com a
    // fila vazia (sem travar).
    q.finish();
    auto item = q.pop();
    check(!item.has_value(), "pop na fila vazia concluida nao bloqueia");

    ThreadSafeQueue q2;
    bool popped = false;
    std::thread t([&q2, &popped] {
        auto it = q2.pop();
        popped = !it.has_value();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    q2.finish();
    t.join();
    check(popped, "consumidor acorda com finish() e retorna vazio");
}

static void test_queue_many_threads()
{
    constexpr int TOTAL = 100000;
    ThreadSafeQueue q;

    std::thread producer([&q] {
        for (int i = 0; i < TOTAL; ++i)
            q.push("x");
        q.finish();
    });

    std::atomic<int> received{0};
    std::vector<std::thread> consumers;
    for (int c = 0; c < 4; ++c)
    {
        consumers.emplace_back([&q, &received] {
            while (auto item = q.pop())
                ++received;
        });
    }

    producer.join();
    for (auto &t : consumers)
        t.join();

    check(received.load() == TOTAL,
          "4 consumidores recebem exatamente todos os itens");
}

// ---------------------------------------------------------------------------
// Extracao de payload
// ---------------------------------------------------------------------------

static void test_extract_single_payload()
{
    auto payloads = extract_payloads("<payload>Fx:1 Vel=62.2</payload>");
    check(payloads.size() == 1, "um payload e extraido");
    check(payloads[0] == "Fx:1 Vel=62.2", "conteudo do payload correto");
}

static void test_extract_multiple_payloads()
{
    auto payloads = extract_payloads("<payload>um</payload>x<payload>dois</payload>");
    check(payloads.size() == 2, "dois payloads sao extraidos");
    check(payloads[0] == "um" && payloads[1] == "dois",
          "payloads extraidos na ordem");
}

static void test_extract_no_tags()
{
    auto payloads = extract_payloads("linha sem tags");
    check(payloads.empty(), "linha sem tags nao gera payloads");
}

static void test_extract_unclosed_tag()
{
    auto payloads = extract_payloads("<payload>conteudo sem fechamento");
    check(payloads.empty(), "tag nao fechada e ignorada");
}

static void test_extract_empty_payload()
{
    auto payloads = extract_payloads("<payload></payload>");
    check(payloads.size() == 1 && payloads[0].empty(),
          "payload vazio e extraido");
}

int main()
{
    test_queue_fifo_order();
    test_queue_pop_blocks_until_done();
    test_queue_many_threads();
    test_extract_single_payload();
    test_extract_multiple_payloads();
    test_extract_no_tags();
    test_extract_unclosed_tag();
    test_extract_empty_payload();

    if (g_failures == 0)
    {
        printf("TODOS OS TESTES PASSARAM\n");
        return 0;
    }
    printf("%d teste(s) FALHOU(FALHARAM)\n", g_failures);
    return 1;
}
