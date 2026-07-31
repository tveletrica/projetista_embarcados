# Q3 - Produtor-Consumidor em C++

Implementacao do cenario produtor-consumidor com duas threads
independentes em C++17.

## Descricao

1. A thread **Produtora** le o arquivo `input.xml` linha a linha e envia
   cada linha para uma fila compartilhada (`ThreadSafeQueue`).

2. A thread **Consumidora** retira as linhas da fila e imprime apenas o
   conteudo que estiver dentro das tags `<payload></payload>`.

A regiao critica (a fila) e protegida com `std::mutex` e `std::condition_variable`:

* `push()` insere um item com o mutex travado e notifica o consumidor.
* `pop()` bloqueia o consumidor via `wait()` ate existir um item na fila
  ou a producao ser concluida (`m_done`), retornando `std::nullopt` ao
  final, o que encerra a thread consumidora de forma ordenada.

O conteudo do payload e extraido com busca textual das tags, sem
dependencia de bibliotecas externas de XML.

## Build e execucao

```bash
make                          # compila o binario q3
./q3 input.xml                # executa com o XML fornecido
./q3 /caminho/outro.xml       # opcional: outro arquivo XML
make clean                    # remove o binario gerado
```

## Testes unitarios

```bash
make test       # compila e executa test_q3
```

Os testes cobrem a `ThreadSafeQueue` (ordem FIFO, encerramento com
`finish()`, consumo com multiplas threads) e a extracao de payload
(simples, multiplo, vazio, ausente e tag nao fechada).

## Saida esperada

Para o `input.xml` fornecido, a saida e apenas:

```
Fx:1 Vel1=62.2; Vel2=62.3; Vel3=62.2; Vel=62.2
```
