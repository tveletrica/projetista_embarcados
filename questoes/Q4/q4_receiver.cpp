// ---------------------------------------------------------------------------
// Processo 2 (receiver)
//
// Recebe as imagens enviadas pelo Processo 1 via socket Unix e as grava em
// disco. Nao realiza nenhum processamento com a imagem.
//
// Uso: ./q4_receiver [arquivo_saida]
// ---------------------------------------------------------------------------

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "q4_protocol.h"

// Le exatamente n bytes do socket. Retorna true em caso de sucesso.
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

int main(int argc, char *argv[])
{
    const std::string output_path = (argc > 1) ? argv[1] : "imagem_recebida.ppm";

    // Cria o socket e registra o endereco no sistema de arquivos.
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        std::cerr << "Erro ao criar socket: " << std::strerror(errno) << '\n';
        return 1;
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, Q4_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(Q4_SOCKET_PATH);

    if (bind(listen_fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        std::cerr << "Erro no bind: " << std::strerror(errno) << '\n';
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 1) < 0)
    {
        std::cerr << "Erro no listen: " << std::strerror(errno) << '\n';
        close(listen_fd);
        return 1;
    }

    std::cout << "Processo 2 aguardando imagens em " << Q4_SOCKET_PATH << "...\n";

    while (true)
    {
        int conn_fd = accept(listen_fd, nullptr, nullptr);
        if (conn_fd < 0)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "Erro no accept: " << std::strerror(errno) << '\n';
            break;
        }

        // 1) Le o cabecalho: tamanho da imagem (protocolo, big-endian).
        uint8_t header[Q4_HEADER_SIZE];
        if (!read_full(conn_fd, header, Q4_HEADER_SIZE))
        {
            std::cerr << "Falha ao ler cabecalho\n";
            close(conn_fd);
            continue;
        }
        uint32_t image_size = decode_size(header);

        // 2) Le exatamente o tamanho informado e grava em disco.
        std::vector<uint8_t> image(image_size);
        if (!read_full(conn_fd, image.data(), image_size))
        {
            std::cerr << "Falha ao ler a imagem\n";
            close(conn_fd);
            continue;
        }

        std::ofstream out(output_path, std::ios::binary);
        if (!out)
        {
            std::cerr << "Erro ao abrir " << output_path << '\n';
            close(conn_fd);
            continue;
        }
        out.write(reinterpret_cast<const char *>(image.data()), image.size());
        out.close();

        std::cout << "Imagem recebida (" << image_size << " bytes) gravada em "
                  << output_path << '\n';

        close(conn_fd);
    }

    close(listen_fd);
    unlink(Q4_SOCKET_PATH);
    return 0;
}
