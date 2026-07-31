# Q1 - Script monitor de recursos

Script bash que verifica a utilizacao de todas as particoes (`/dev/*`),
o uso de memoria RAM e a temperatura dos cores do sistema, enviando um
e-mail de alerta e gravando em log toda variavel acima dos limites.

## Requisitos

* Linux com bash, `df`, `free` e `date` (pacotes base).
* Cliente de e-mail: `mail`, `mailx` ou `sendmail` (opcional, o script
  detecta automaticamente qual esta instalado).

## Limites e configuracoes

Edite o topo do script `monitor.sh`:

| Variavel       | Default        | Descricao                        |
| -------------- | -------------- | -------------------------------- |
| `LIMIT_DISK`   | `80`           | % de uso de disco por particao   |
| `LIMIT_RAM`    | `90`           | % de uso de memoria RAM          |
| `LIMIT_TEMP`   | `75`           | temperatura maxima dos cores (C) |
| `MAIL_TO`      | `root@localhost` | destinatario dos alertas       |
| `LOG_FILE`     | `/var/log/monitor_recursos.log` | arquivo de log |

O log e gravado por redirecionamento direto de saida (`>> LOG_FILE`),
sem depender do rsyslog.

## Execucao manual

```bash
chmod +x monitor.sh

# Teste sem enviar e-mail nem gravar log (apenas imprime):
./monitor.sh --dry-run

# Grava apenas no log, sem enviar e-mails:
./monitor.sh --log-only --file /tmp/monitor.log

# Modo normal:
./monitor.sh
```

## Testes unitarios

```bash
chmod +x test_monitor.sh
./test_monitor.sh
```

O teste executa o script com comandos falsos (`df`, `free`, `sensors`,
`date`, `mail`) em um PATH temporario, cobrindo: alertas de disco/RAM/
temperatura acima dos limites, envio de e-mail com detalhes no corpo e
ausencia de alertas com uso normal.

## Agendamento no cron

Adicione a crontab do usuario root:

```cron
*/5 * * * * /caminho/completo/questoes/Q1/monitor.sh
```

## Logs

Cada execucao registra inicio e fim do monitoramento, alem dos alertas:

```
[2026-07-31 11:08:45] INFO: Iniciando monitoramento (limites: disco 80%, RAM 90%, temp 75C)
[2026-07-31 11:08:45] ALERTA: ALERTA MOBIT - Utilizacao de disco acima do limite
[2026-07-31 11:08:46] INFO: Monitoramento concluido
```

## Observacoes

* A temperatura e lida de `/sys/class/thermal/thermal_zone*/temp` e,
  quando disponivel, do comando `sensors` (fallback para cores da CPU).
* Se nao houver cliente de e-mail instalado, o script registra o alerta
  no log e reporta o erro (nao falha silenciosamente).
