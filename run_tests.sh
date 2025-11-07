#!/bin/bash
# Script de Teste Final do Compilador

echo "========================================"
echo "COMPILADOR DE LINGUAGEM MATEMÁTICA"
echo "Teste de Validação Final"
echo "========================================"
echo ""

TESTS=(
    "tests/hello_world.mf"
    "tests/variables.mf"
    "tests/arrays.mf"
    "tests/control.mf"
    "tests/media.mf"
    "tests/complex_functions.mf"
    "tests/complex_numbers.mf"
    "tests/math_functions.mf"
    "tests/advanced_types.mf"
    "tests/break_continue.mf"
    "tests/try_catch_const.mf"
    "tests/all_operators.mf"
    "tests/special_literals.mf"
    "tests/math_operators.mf"
    "tests/complex_nested.mf"
    "tests/aluno.mf"
    "tests/conjuntos.mf"
    "tests/soma_recursiva.mf"
    "tests/teste_misturado.mf"
)

PASSED=0
FAILED=0

for test_file in "${TESTS[@]}"; do
    if [ ! -f "$test_file" ]; then
        echo "⚠️  Arquivo não encontrado: $test_file"
        continue
    fi

    echo -n "Testando $(basename $test_file)... "

    output=$(./mathc "$test_file" 2>&1)

    if echo "$output" | grep -q "SUCESSO"; then
        echo "✅ PASSOU"
        ((PASSED++))
    else
        echo "❌ FALHOU"
        ((FAILED++))
    fi
done

echo ""
echo "========================================"
echo "RESULTADO FINAL"
echo "========================================"
echo "✅ Testes aprovados: $PASSED"
echo "❌ Testes falhados: $FAILED"
echo "========================================"

if [ $FAILED -eq 0 ]; then
    echo "🎉 TODOS OS TESTES PASSARAM!"
    exit 0
else
    echo "⚠️  Alguns testes falharam"
    exit 1
fi

