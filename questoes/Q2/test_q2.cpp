// ---------------------------------------------------------------------------
// Testes unitarios do Q2 (montagem da mensagem logada).
//
// Para testar as funcoes estaticas de q2.cpp sem executar o main(), o
// arquivo e incluido diretamente com o main renomeado via macro.
// ---------------------------------------------------------------------------

#define main q2_original_main
#include "q2.cpp"
#undef main

#include <cstdio>
#include <string>

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

static void test_message_format()
{
    const std::string msg = build_message(3);

    const std::string expected =
        "--------------------\n"
        "Iniciando bloco 3\n"
        "Hello world from thread 3\n"
        "Fim do bloco 3\n"
        "--------------------\n";

    check(msg == expected, "mensagem com formato exato para id=3");

    const std::string msg2 = build_message(11);
    check(msg2.find("Iniciando bloco 11") != std::string::npos,
          "mensagem contem id 11");
    check(msg2.find("Hello world from thread 11") != std::string::npos,
          "mensagem contem thread 11");
    check(msg2.find("Fim do bloco 11") != std::string::npos,
          "mensagem contem fim do bloco 11");
}

static void test_message_boundaries()
{
    const std::string msg = build_message(0);
    check(msg.front() == '-', "mensagem comeca com separador");
    check(msg.back() == '\n', "mensagem termina com quebra de linha");
    check(msg.find("Iniciando bloco 0") < msg.find("Hello world"),
          "inicio do bloco vem antes do hello");
    check(msg.find("Hello world") < msg.find("Fim do bloco"),
          "hello vem antes do fim do bloco");
}

int main()
{
    test_message_format();
    test_message_boundaries();

    if (g_failures == 0)
    {
        printf("TODOS OS TESTES PASSARAM\n");
        return 0;
    }
    printf("%d teste(s) FALHOU(FALHARAM)\n", g_failures);
    return 1;
}
