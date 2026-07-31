// ---------------------------------------------------------------------------
// Processo 1 (sender)
//
// Simula a leitura de um frame de camera lendo um arquivo de imagem em
// disco e o envia ao Processo 2 pelo socket Unix, seguindo o protocolo:
// primeiro informa o tamanho da imagem, depois envia os dados.
//
// Uso: ./q4_sender [arquivo_imagem] [numero_de_frames]
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

// Envia exatamente n bytes pelo socket. Retorna true em caso de sucesso.
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

int main(int argc, char *argv[])
{
    const std::string image_path = (argc > 1) ? argv[1] : "imagem_camera.ppm";
    const int frame_count = (argc > 2) ? std::stoi(argv[2]) : 1;

    // "Leitura da camera": carrega a imagem do arquivo em disco.
    std::ifstream in(image_path, std::ios::binary | std::ios::ate);
    if (!in)
    {
        std::cerr << "Erro ao abrir a imagem " << image_path << '\n';
        return 1;
    }
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    std::vector<uint8_t> image(static_cast<size_t>(size));
    in.read(reinterpret_cast<char *>(image.data()), size);
    in.close();

    std::cout << "Processo 1: imagem " << image_path << " carregada ("
              << image.size() << " bytes)\n";

    // Cada frame e enviado em uma conexao dedicada: o Processo 2 trata cada
    // conexao como um unico frame.
    uint8_t header[Q4_HEADER_SIZE];
    for (int frame = 1; frame <= frame_count; ++frame)
    {
        int fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0)
        {
            std::cerr << "Erro ao criar socket: " << std::strerror(errno) << '\n';
            return 1;
        }

        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, Q4_SOCKET_PATH, sizeof(addr.sun_path) - 1);

        if (connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            std::cerr << "Erro ao conectar: " << std::strerror(errno)
                      << "\nVerifique se o Processo 2 esta rodando.\n";
            close(fd);
            return 1;
        }

        encode_size(static_cast<uint32_t>(image.size()), header);

        if (!write_full(fd, header, Q4_HEADER_SIZE))
        {
            std::cerr << "Falha ao enviar o cabecalho do frame " << frame << '\n';
            close(fd);
            break;
        }
        if (!write_full(fd, image.data(), image.size()))
        {
            std::cerr << "Falha ao enviar o frame " << frame << '\n';
            close(fd);
            break;
        }
        std::cout << "Frame " << frame << " enviado (" << image.size()
                  << " bytes)\n";

        close(fd);
        usleep(100000);
    }

    return 0;
}
