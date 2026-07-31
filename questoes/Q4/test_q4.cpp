// ---------------------------------------------------------------------------
// Testes unitarios do Q4 (protocolo de comunicacao entre processos).
//
// Verifica a serializacao big-endian do cabecalho e a transferencia de
// dados ponta a ponta usando socketpair(AF_UNIX) com fork, seguindo o
// protocolo [4 bytes tamanho][N bytes dados] definido em q4_protocol.h.
// ---------------------------------------------------------------------------

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include "q4_protocol.h"

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
// encode/decode do cabecalho
// ---------------------------------------------------------------------------

static void test_encode_decode_roundtrip()
{
    const uint32_t sizes[] = {0, 1, 12345, 0x7FFFFFFF, 0xFFFFFFFF};
    bool ok = true;
    for (uint32_t s : sizes)
    {
        uint8_t header[Q4_HEADER_SIZE];
        encode_size(s, header);
        ok = ok && (decode_size(header) == s);
    }
    check(ok, "roundtrip encode/decode para tamanhos limites");
}

static void test_big_endian_encoding()
{
    uint8_t header[Q4_HEADER_SIZE];
    encode_size(0xDEADBEEF, header);
    check(header[0] == 0xDE && header[1] == 0xAD &&
              header[2] == 0xBE && header[3] == 0xEF,
          "codificacao big-endian explicita (MSB primeiro)");
}

// ---------------------------------------------------------------------------
// Transferencia de dados com o protocolo (fork + socketpair)
// ---------------------------------------------------------------------------

static bool read_full(int fd, uint8_t *buf, size_t n)
{
    size_t received = 0;
    while (received < n)
    {
        ssize_t r = recv(fd, buf + received, n - received, 0);
        if (r <= 0)
            return false;
        received += static_cast<size_t>(r);
    }
    return true;
}

static bool write_full(int fd, const uint8_t *buf, size_t n)
{
    size_t sent = 0;
    while (sent < n)
    {
        ssize_t w = send(fd, buf + sent, n - sent, MSG_NOSIGNAL);
        if (w < 0)
        {
            if (errno == EINTR)
                continue;
            return false;
        }
        sent += static_cast<size_t>(w);
    }
    return true;
}

static bool transfer_with_protocol(const std::vector<uint8_t> &payload)
{
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
        return false;

    pid_t pid = fork();
    if (pid < 0)
    {
        close(sv[0]);
        close(sv[1]);
        return false;
    }

    if (pid == 0)
    {
        // Processo "sender": cabecalho + dados.
        close(sv[0]);
        uint8_t header[Q4_HEADER_SIZE];
        encode_size(static_cast<uint32_t>(payload.size()), header);
        bool ok = write_full(sv[1], header, Q4_HEADER_SIZE) &&
                  write_full(sv[1], payload.data(), payload.size());
        close(sv[1]);
        _exit(ok ? 0 : 1);
    }

    // Processo "receiver": le cabecalho, depois exatamente N bytes.
    close(sv[1]);
    uint8_t header[Q4_HEADER_SIZE];
    bool ok = read_full(sv[0], header, Q4_HEADER_SIZE);
    uint32_t size = 0;
    if (ok)
        size = decode_size(header);
    ok = ok && size == payload.size();

    std::vector<uint8_t> data(size);
    ok = ok && read_full(sv[0], data.data(), size);
    ok = ok && data == payload;

    close(sv[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    return ok && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static void test_transfer_payloads()
{
    // Dados de tamanhos diferentes (poucos bytes e um frame grande).
    const std::vector<uint8_t> small = {'P', '6', '\n'};
    std::vector<uint8_t> large(12301);
    for (size_t i = 0; i < large.size(); ++i)
        large[i] = static_cast<uint8_t>(i * 7);

    check(transfer_with_protocol(small),
          "transferencia de payload pequeno via socketpair");
    check(transfer_with_protocol(large),
          "transferencia de payload de 12301 bytes via socketpair");
}

int main()
{
    test_encode_decode_roundtrip();
    test_big_endian_encoding();
    test_transfer_payloads();

    if (g_failures == 0)
    {
        printf("TODOS OS TESTES PASSARAM\n");
        return 0;
    }
    printf("%d teste(s) FALHOU(FALHARAM)\n", g_failures);
    return 1;
}
