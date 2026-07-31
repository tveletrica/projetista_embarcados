#ifndef Q4_PROTOCOL_H
#define Q4_PROTOCOL_H

#include <cstddef>
#include <cstdint>

// ---------------------------------------------------------------------------
// Protocolo simples para envio de imagens entre os processos:
//
//   [4 bytes: tamanho (uint32, big-endian)] [N bytes: dados da imagem]
//
// O Processo 1 (sender) informa o tamanho da imagem antes de enviar os
// dados. O Processo 2 (receiver) le o cabecalho e entao recebe exatamente
// N bytes, gravando-os em disco. Isso delimita cada frame na conexao.
// ---------------------------------------------------------------------------

// Endereco do socket Unix usado como canal de IPC.
constexpr const char *Q4_SOCKET_PATH = "/tmp/mobit_q4_image.sock";

// Tamanho do cabecalho de protocolo (uint32 big-endian).
constexpr size_t Q4_HEADER_SIZE = sizeof(uint32_t);

// Serializa o tamanho em big-endian (network byte order).
inline void encode_size(uint32_t size, uint8_t out[Q4_HEADER_SIZE])
{
    out[0] = static_cast<uint8_t>((size >> 24) & 0xFF);
    out[1] = static_cast<uint8_t>((size >> 16) & 0xFF);
    out[2] = static_cast<uint8_t>((size >> 8) & 0xFF);
    out[3] = static_cast<uint8_t>(size & 0xFF);
}

// Deserializa o tamanho lido do socket.
inline uint32_t decode_size(const uint8_t in[Q4_HEADER_SIZE])
{
    return (static_cast<uint32_t>(in[0]) << 24) |
           (static_cast<uint32_t>(in[1]) << 16) |
           (static_cast<uint32_t>(in[2]) << 8) |
           static_cast<uint32_t>(in[3]);
}

#endif // Q4_PROTOCOL_H
