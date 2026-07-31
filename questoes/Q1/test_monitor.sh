#!/bin/bash
#
# test_monitor.sh - Testes unitarios do monitor.sh
#
# Executa o monitor.sh com comandos falsos (df/free/sensors/date/mail) em um
# diretorio temporario no PATH, simulando cenarios de alerta e de normalidade
# sem depender do estado real da maquina.
#
# Uso: ./test_monitor.sh
#
# Cenario 1: uso acima dos limites -> alertas de disco, RAM e temperatura
#            gravados no log (somente assuntos; detalhes vao no e-mail).
# Cenario 2: envio de e-mail com detalhes no corpo da mensagem.
# Cenario 3: uso normal -> nenhum alerta.
#
# Obs.: as zonas termicas reais (/sys/class/thermal) sao lidas pelo script;
# no cenario 3 uma copia com LIMIT_TEMP alto neutraliza a maquina real.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MONITOR="$SCRIPT_DIR/monitor.sh"

TMP="$(mktemp -d)"
FAKE_BIN="$TMP/bin"
MAIL_OUT="$TMP/emails.txt"
mkdir -p "$FAKE_BIN"

PASS=0
FAIL=0

pass() { echo "[OK] $1"; PASS=$((PASS + 1)); }
fail() { echo "[FALHOU] $1"; FAIL=$((FAIL + 1)); }

cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

# ---------------------------------------------------------------------------
# Ferramentas falsas
# ---------------------------------------------------------------------------

cat > "$FAKE_BIN/date" <<'EOF'
#!/bin/bash
echo "2026-07-31 12:00:00"
EOF

# mail: captura o assunto e o corpo em vez de enviar de verdade.
cat > "$FAKE_BIN/mail" <<EOF
#!/bin/bash
{
    echo "ARGS: \$@"
    echo "---"
    cat
    echo "===="
} >> "$MAIL_OUT"
exit 0
EOF

chmod +x "$FAKE_BIN"/*

export PATH="$FAKE_BIN:$PATH"
export MAIL_OUT

write_fakes() { # $1=df $2=free $3=sensors
    cat > "$FAKE_BIN/df" <<EOF
#!/bin/bash
cat <<'DF'
$1
DF
EOF

    cat > "$FAKE_BIN/free" <<EOF
#!/bin/bash
cat <<'FREE'
$2
FREE
EOF

    cat > "$FAKE_BIN/sensors" <<EOF
#!/bin/bash
cat <<'SENS'
$3
SENS
EOF

    chmod +x "$FAKE_BIN/df" "$FAKE_BIN/free" "$FAKE_BIN/sensors"
}

DF_ALERT='Filesystem     1K-blocks    Used Available Use% Mounted on
/dev/sda1        1000000  950000     50000  95% /
/dev/sdb1        1000000  100000    900000  10% /mnt/dados'

FREE_ALERT='              total        used        free      shared  buff/cache   available
Mem:           8000        7900         100         0           0         100
Swap:          2000           0        2000'

SENS_ALERT='Core 0:       +88.0°C
Core 1:       +45.0°C'

DF_NORMAL='Filesystem     1K-blocks    Used Available Use% Mounted on
/dev/sda1        1000000  200000    800000  20% /'

FREE_NORMAL='              total        used        free      shared  buff/cache   available
Mem:           8000         500        7500         0           0         500
Swap:          2000           0        2000'

SENS_NORMAL='Core 0:       +45.0°C'

# ---------------------------------------------------------------------------
# Cenario 1: uso acima dos limites -> alertas no log (sem e-mail)
# ---------------------------------------------------------------------------

write_fakes "$DF_ALERT" "$FREE_ALERT" "$SENS_ALERT"

LOG1="$TMP/monitor_alerta.log"
"$MONITOR" --log-only --file "$LOG1" >/dev/null 2>&1

grep -q "Utilizacao de disco acima do limite" "$LOG1" \
    && pass "Cenario 1: alerta de disco gravado no log" \
    || fail "Cenario 1: alerta de disco nao encontrado no log"

grep -q "Memoria RAM acima do limite" "$LOG1" \
    && pass "Cenario 1: alerta de RAM gravado no log" \
    || fail "Cenario 1: alerta de RAM nao encontrado no log"

grep -q "Temperatura acima do limite" "$LOG1" \
    && pass "Cenario 1: alerta de temperatura gravado no log" \
    || fail "Cenario 1: alerta de temperatura nao encontrado no log"

# ---------------------------------------------------------------------------
# Cenario 2: envio de e-mail com detalhes no corpo da mensagem
# ---------------------------------------------------------------------------

LOG2="$TMP/monitor_email.log"
"$MONITOR" --file "$LOG2" >/dev/null 2>&1

grep -q "ALERTA MOBIT - Utilizacao de disco" "$MAIL_OUT" \
    && pass "Cenario 2: e-mail de alerta de disco enviado" \
    || fail "Cenario 2: e-mail de alerta de disco nao enviado"

grep -q "Particao /dev/sda1 (/): 95% de utilizacao" "$MAIL_OUT" \
    && pass "Cenario 2: corpo do e-mail traz o detalhe da particao" \
    || fail "Cenario 2: detalhe da particao ausente no e-mail"

grep -q "Memoria RAM: 7900 MB usados de 8000 MB (98%)" "$MAIL_OUT" \
    && pass "Cenario 2: corpo do e-mail traz o detalhe da RAM" \
    || fail "Cenario 2: detalhe da RAM ausente no e-mail"

grep -q "Core 0: 88C" "$MAIL_OUT" \
    && pass "Cenario 2: corpo do e-mail traz o detalhe do core quente" \
    || fail "Cenario 2: detalhe do core ausente no e-mail"

grep -q "EMAIL" "$LOG2" \
    && pass "Cenario 2: envio de e-mail registrado no log" \
    || fail "Cenario 2: envio de e-mail nao registrado no log"

# ---------------------------------------------------------------------------
# Cenario 3: uso normal -> nenhum alerta
# ---------------------------------------------------------------------------

write_fakes "$DF_NORMAL" "$FREE_NORMAL" "$SENS_NORMAL"

# Copia do script com LIMIT_TEMP alto para neutralizar as zonas termicas
# reais da maquina (/sys/class/thermal) no cenario de normalidade.
sed 's/^LIMIT_TEMP=75$/LIMIT_TEMP=500/' "$MONITOR" > "$TMP/monitor_normal.sh"
chmod +x "$TMP/monitor_normal.sh"

LOG3="$TMP/monitor_normal.log"
"$TMP/monitor_normal.sh" --log-only --file "$LOG3" >/dev/null 2>&1

grep -q "ALERTA" "$LOG3" \
    && fail "Cenario 3: alerta inesperado com uso normal" \
    || pass "Cenario 3: nenhum alerta com uso normal"

grep -q "Iniciando monitoramento" "$LOG3" && \
grep -q "Monitoramento concluido" "$LOG3" \
    && pass "Cenario 3: inicio e fim registrados no log" \
    || fail "Cenario 3: inicio/fim ausentes no log"

# ---------------------------------------------------------------------------
# Sintaxe do script
# ---------------------------------------------------------------------------

bash -n "$MONITOR" \
    && pass "Sintaxe do monitor.sh valida" \
    || fail "Sintaxe do monitor.sh invalida"

# ---------------------------------------------------------------------------

echo
echo "Resultado: $PASS passou, $FAIL falhou"
[ "$FAIL" -eq 0 ]
