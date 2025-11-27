#!/bin/bash
# Script de Teste Final do Compilador

# Define o diretório de output padrão se não estiver definido
export MATHC_OUTPUT_DIR="${MATHC_OUTPUT_DIR:-build/tests}"

echo "========================================"
echo "COMPILADOR DE LINGUAGEM MATEMÁTICA"
echo "Teste de Validação Final"
echo "Diretório de output: $MATHC_OUTPUT_DIR"
echo "========================================"
echo ""

# Verifica se o diretório tests existe
if [ ! -d "tests" ]; then
    echo "❌ Diretório 'tests' não encontrado!"
    exit 1
fi

# Encontra todos os arquivos .mf em tests (não recursivo)
mapfile -t TEST_FILES < <(find tests -maxdepth 1 -type f -name "*.mf" | sort)

if [ ${#TEST_FILES[@]} -eq 0 ]; then
    echo "⚠️  Nenhum arquivo .mf encontrado em 'tests/'"
    exit 1
fi

PASSED=0
FAILED=0
declare -a FAILED_DETAILS  # Array para armazenar detalhes dos falhos

# Determina o comprimento máximo do nome base dos arquivos para alinhar a saída
MAX_LEN=0
for test_file in "${TEST_FILES[@]}"; do
    base_name=$(basename "$test_file")
    basename_len=${#base_name}
    if (( basename_len > MAX_LEN )); then
        MAX_LEN=$basename_len
    fi
done

if (( MAX_LEN < 20 )); then
    MAX_LEN=20
fi

# Cabeçalho da tabela de testes
printf "%-${MAX_LEN}s  %s\n" "ARQUIVO" "RESULTADO"
printf "%-${MAX_LEN}s  %s\n" "$(printf '%*s' $MAX_LEN | tr ' ' '-')" "--------"

for test_file in "${TEST_FILES[@]}"; do
    base_name=$(basename "$test_file" .mf)
    printf "%-${MAX_LEN}s  " "$base_name.mf"

    # Arquivo executável esperado
    expected_exe="$MATHC_OUTPUT_DIR/$base_name.out"

    # Usa arquivo temporário para capturar saída exata
    temp_out=$(mktemp)
    if ./mathc "$test_file" >"$temp_out" 2>&1; then
        # Compilador retornou 0 - verifica se executável foi gerado
        if [ -f "$expected_exe" ]; then
            # Executa o programa compilado
            if "$expected_exe" >>"$temp_out" 2>&1; then
                # Programa executou com sucesso
                echo "✅ PASSOU"
                ((PASSED++))
            else
                # Programa executou mas falhou (exit != 0)
                echo "❌ FALHOU"
                ((FAILED++))
                FAILED_DETAILS+=("$base_name.mf|$temp_out")
            fi
        else
            # Executável não foi gerado
            echo "❌ FALHOU"
            ((FAILED++))
            FAILED_DETAILS+=("$base_name.mf|$temp_out")
        fi
    else
        # Compilador falhou (exit != 0) → falha
        echo "❌ FALHOU"
        ((FAILED++))
        FAILED_DETAILS+=("$base_name|$temp_out")
    fi
done

echo ""
echo "========================================"
echo "RESULTADO FINAL"
echo "========================================"
printf "✅ Testes aprovados: %2d\n" $PASSED
printf "❌ Testes falhados:  %2d\n" $FAILED
printf "📄 Total de testes:  %2d\n" ${#TEST_FILES[@]}
echo "========================================"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "🔍 DETALHES DOS TESTES FALHOS:"
    echo "========================================"
    for detail in "${FAILED_DETAILS[@]}"; do
        IFS='|' read -r file temp_file <<< "$detail"
        echo ">️  Arquivo: $file"
        echo "   ┌──────────────────────────────────────"
        head -n 3 "$temp_file" | sed 's/^/   │ /'
        echo "   └──────────────────────────────────────"
        echo ""
        rm -f "$temp_file"
    done
    exit 1
else
    echo "🎉 TODOS OS TESTES PASSARAM!"
    exit 0
fi