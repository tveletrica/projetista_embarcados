# Q2 - Sincronizando o uso do log com varias threads

Correcao do codigo disponibilizado, que inicializa 10 threads, cada uma
responsavel por logar uma mensagem. Os problemas corrigidos:

## Correcoes

1. **Makefile** - criado para compilar o projeto com um unico `make` e
   remover o binario com `make clean`.

2. **Mensagens fora de sincronia (2.2)** - as threads interrompiam as
   mensagens umas das outras na saida padrao. A solucao monta a mensagem
   completa em uma `std::string` (via `std::ostringstream`) e a envia em
   uma unica operacao. O envio ao syslog e protegido por um `std::mutex`
   (`g_log_mutex`), garantindo que cada mensagem saia do inicio ao fim
   sem interrupcoes.

3. **Log no syslog (2.3)** - a saida padrao foi substituida por `syslog(3)`:
   `openlog("q2", LOG_PID | LOG_CONS, LOG_USER)` no inicio do programa e
   `closelog()` ao final. As mensagens sao enviadas com prioridade
   `LOG_INFO`.

## Build e execucao

```bash
make            # compila o binario q2
./q2            # executa, enviando as 10 mensagens ao syslog
make clean      # remove o binario gerado
```

## Testes unitarios

```bash
make test       # compila e executa test_q2 (formato da mensagem)
```

Os testes verificam a montagem da mensagem (`build_message`) sem depender
do syslog, incluindo formato exato e ordem das linhas.

## Verificando o log

As mensagens podem ser conferidas com:

```bash
tail -f /var/log/syslog        # Ubuntu/Debian com rsyslog
journalctl -t q2 -f            # sistemas com systemd
```
