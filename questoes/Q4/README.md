# Q4 - Comunicacao inter processos

Simulacao de leitura de imagem de camera em tempo real (Processo 1) com
envio para um segundo processo (Processo 2), que grava a imagem recebida
em disco sem realizar processamento.

## Mecanismo de IPC escolhido: Socket Unix (`SOCK_STREAM`)

Justificativa para o cenario de video em tempo real:

* Permite enviar imagens de tamanho arbitrario, com delimitacao confiavel
  via cabecalho de protocolo (diferente de pipe com dados de tamanho fixo).
* Ordena e entrega os dados sem perda, como exige um stream de video.
* Suporta multiplos clientes/quadros e e o mesmo mecanismo usado por
  aplicacoes reais de captura de camera.
* Nao depende do sistema de arquivos para trafegar os dados (restricao
  temporal da aplicacao).

Alternativas validas: `pipe` nomeado (FIFO) e memoria compartilhada com
semafaros. A escolha do socket foi feita pela robustez para fluxo de dados
contínuo e de tamanho variavel.

## Protocolo

```
[4 bytes: tamanho (uint32, big-endian)] [N bytes: dados da imagem]
```

O Processo 1 informa o tamanho da imagem antes de enviar os dados
(requisito 4). O Processo 2 le o cabecalho, recebe exatamente N bytes e
grava em disco. Cada conexao carrega um unico frame (1 conexao = 1 imagem),
delimitando os quadros no fluxo.

O protocolo esta documentado em `q4_protocol.h` (codificacao big-endian
explícita, portavel entre arquiteturas).

## Build

```bash
make              # compila q4_receiver e q4_sender
make sample       # gera a imagem de exemplo imagem_camera.ppm
```

## Testes unitarios

```bash
make test         # compila e executa test_q4
```

Os testes verificam o encode/decode big-endian do cabecalho (roundtrip e
ordem dos bytes) e a transferencia ponta a ponta do protocolo via
`socketpair` com fork, para payloads pequeno e de 12301 bytes.

## Execucao

Terminal 1 (Processo 2 - receptor):

```bash
./q4_receiver saida.ppm
```

Terminal 2 (Processo 1 - remetente/simulacao da camera):

```bash
./q4_sender imagem_camera.ppm 3
```

O sender envia 3 frames. A cada frame, o receptor imprime
`Imagem recebida (N bytes) gravada em saida.ppm`.

Para conferir a integridade:

```bash
cmp imagem_camera.ppm saida.ppm && echo "IMAGENS IDENTICAS"
```

### Uso

```
./q4_receiver [arquivo_saida]        # default: imagem_recebida.ppm
./q4_sender [arquivo_imagem] [frames]  # default: imagem_camera.ppm, 1
```

O endereco do socket (`/tmp/mobit_q4_image.sock`) e definido em
`q4_protocol.h`.

## Limpeza

```bash
make clean        # remove os binarios
```
