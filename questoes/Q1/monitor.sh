#!/bin/bash
#
# monitor.sh - Monitor de recursos do sistema
#
# Verifica a porcentagem de utilizacao das particoes (/dev/*), o uso de
# memoria RAM e a temperatura dos cores do sistema. Para cada variavel
# acima do limite configurado, envia um e-mail de alerta e registra a
# ocorrencia no arquivo de log.
#
# Projetado para ser executado pela crontab do sistema, ex.:
#
#     */5 * * * * /caminho/para/questoes/Q1/monitor.sh
#
# Uso:
#     ./monitor.sh [--dry-run] [--log-only] [--file <arquivo_log>]
#
# Opcoes:
#     --dry-run   Nao envia e-mails nem escreve no log (apenas imprime).
#     --log-only  Registra no log sem enviar e-mails.
#     --file      Caminho alternativo para o arquivo de log.

# ---------------------------------------------------------------------------
# Configuracao (limites)
# ---------------------------------------------------------------------------

# Limite de utilizacao de disco (em %) para cada particao /dev/*
LIMIT_DISK=80

# Limite de utilizacao de memoria RAM (em %)
LIMIT_RAM=90

# Limite de temperatura dos cores (em graus Celsius)
LIMIT_TEMP=75

# Destinatario dos e-mails de alerta
MAIL_TO="root@localhost"

# Arquivo de log (redirecionamento direto de saida, sem rsyslog)
LOG_FILE="/var/log/monitor_recursos.log"

# Comando para envio de e-mail
MAIL_CMD=""

# ---------------------------------------------------------------------------
# Fim da configuracao
# ---------------------------------------------------------------------------

set -o pipefail

DRY_RUN=0
LOG_ONLY=0

usage() {
    sed -n '2,20p' "$0" | sed 's/^# \{0,1\}//'
    exit 0
}

log_msg() {
    local level="$1"
    local msg="$2"
    local ts
    ts="$(date '+%Y-%m-%d %H:%M:%S')"
    if [ "$DRY_RUN" -eq 1 ]; then
        printf '[%s] %s: %s\n' "$ts" "$level" "$msg"
    else
        printf '[%s] %s: %s\n' "$ts" "$level" "$msg" >> "$LOG_FILE" 2>/dev/null || \
            printf '[%s] %s: %s\n' "$ts" "$level" "$msg"
    fi
}

send_alert() {
    local subject="$1"
    local body="$2"
    if [ "$DRY_RUN" -eq 1 ] || [ "$LOG_ONLY" -eq 1 ]; then
        log_msg "ALERTA" "$subject"
        return 0
    fi
    if [ -z "$MAIL_CMD" ]; then
        log_msg "ERRO" "Nenhum cliente de e-mail encontrado (mail/mailx/sendmail)."
        return 1
    fi
    # shellcheck disable=SC2086
    printf '%s\n' "$body" | $MAIL_CMD -s "$subject" "$MAIL_TO" 2>/dev/null
    if [ $? -eq 0 ]; then
        log_msg "EMAIL" "Alerta enviado para $MAIL_TO: $subject"
    else
        log_msg "ERRO" "Falha ao enviar e-mail: $subject"
    fi
}

# ---------------------------------------------------------------------------
# Utilizacao das particoes /dev/*
# ---------------------------------------------------------------------------

check_disk() {
    local part usage mounted
    local report=""

    # Exclui tmpfs, loop e arquivos de swap montados em /dev/*
    while read -r part usage mounted; do
        [ -n "$part" ] || continue
        usage="${usage%\%}"
        if [ "$usage" -ge "$LIMIT_DISK" ]; then
            report+="Particao $part ($mounted): ${usage}% de utilizacao (limite: ${LIMIT_DISK}%)
"
        fi
    done < <(df -P -t ext4 -t ext3 -t xfs -t btrfs -t vfat 2>/dev/null | grep '/dev/' | awk '{print $1, $5, $6}')

    if [ -n "$report" ]; then
        send_alert "ALERTA MOBIT - Utilizacao de disco acima do limite" \
                   "As seguintes particoes estao acima de ${LIMIT_DISK}%:
${report}"
    fi
}

# ---------------------------------------------------------------------------
# Uso de memoria RAM
# ---------------------------------------------------------------------------

check_ram() {
    local total used pct report
    read -r total used < <(free -m | awk '/^Mem/ {print $2, $3}')
    [ -n "$total" ] || return 1
    pct=$((used * 100 / total))
    if [ "$pct" -ge "$LIMIT_RAM" ]; then
        report="Memoria RAM: ${used} MB usados de ${total} MB (${pct}%), limite: ${LIMIT_RAM}%"
        send_alert "ALERTA MOBIT - Memoria RAM acima do limite" "$report"
    fi
}

# ---------------------------------------------------------------------------
# Temperatura dos cores do sistema
# ---------------------------------------------------------------------------

check_temp() {
    local zone core t
    local report=""
    local count=0

    # 1) /sys/class/thermal/thermal_zone*/temp (contem a temperatura em
    #    miligraus). Usa a maior temperatura encontrada como referencia.
    for zone in /sys/class/thermal/thermal_zone*/temp; do
        [ -r "$zone" ] || continue
        t=$(cat "$zone" 2>/dev/null) || continue
        t=$((t / 1000))
        if [ "$t" -ge "$LIMIT_TEMP" ]; then
            report+="$(basename "$(dirname "$zone")"): ${t}C (limite: ${LIMIT_TEMP}C)
"
        fi
    done

    # 2) sensors (fallback para cores de CPU quando disponivel)
    if command -v sensors >/dev/null 2>&1; then
        while read -r line; do
            [ -n "$line" ] || continue
            core="${line%%:*}"
            t="${line##*:}"
            t=$(printf '%s' "$t" | grep -oE '[0-9]+' | head -1)
            [ -n "$t" ] || continue
            count=$((count + 1))
            if [ "$t" -ge "$LIMIT_TEMP" ]; then
                report+="Core ${core}: ${t}C (limite: ${LIMIT_TEMP}C)
"
            fi
        done < <(sensors 2>/dev/null | grep -iE 'core|package|tctl' | grep -E ':' )
    fi

    if [ -n "$report" ]; then
        send_alert "ALERTA MOBIT - Temperatura acima do limite" \
                   "As seguintes variaveis termicas estao acima de ${LIMIT_TEMP}C:
${report}"
    fi
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

while [ $# -gt 0 ]; do
    case "$1" in
        --dry-run) DRY_RUN=1 ;;
        --log-only) LOG_ONLY=1 ;;
        --file) LOG_FILE="$2"; shift ;;
        -h|--help) usage ;;
        *) printf 'Opcao desconhecida: %s\n' "$1" >&2; usage ;;
    esac
    shift
done

# Seleciona um cliente de e-mail disponivel
for c in mail mailx sendmail; do
    if command -v "$c" >/dev/null 2>&1; then
        MAIL_CMD="$c"
        break
    fi
done

log_msg "INFO" "Iniciando monitoramento (limites: disco ${LIMIT_DISK}%, RAM ${LIMIT_RAM}%, temp ${LIMIT_TEMP}C)"
check_disk
check_ram
check_temp
log_msg "INFO" "Monitoramento concluido"
