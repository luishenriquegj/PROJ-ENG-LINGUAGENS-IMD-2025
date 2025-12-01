# Projeto do Compilador (Entrega Final)

-Implementação do compilador contendo:
(1) Todos os arquivos da implementação do compilador (.l, .y, etc);
(2) Todos os códigos-fonte dos programas solicitados (escritos na linguagem do grupo, ver anexo
problemas_avaliacao.pdf).

OBS.: apenas programas escritos na linguagem do grupo e traduzidos sem erros por seu compilador para C reduzido serão
aceitos.

- Documentação do compilador contendo no mínimo (em PDF):
  (1) Apresentação da linguagem proposta;
  (2) Design da implementação:
  (A) Transformação do código-fonte em unidades léxicas;
  (B) Representação de símbolos, tabela de símbolos e funções associadas;
  (C) Tratamento de estruturas condicionais e de repetição;
  (D) Tratamento de subprogramas;
  (E) Verificações realizadas (tipos, faixas, declaração em duplicidade, etc).
  (3) Instruções de uso do compilador.

Os arquivos relacionados à IMPLEMENTAÇÃO e à DOCUMENTAÇÃO devem ser submetidos via tarefas específicas, apresentadas
neste tópico de aula. Todos os grupos devem submeter seus arquivos utilizando estas tarefas, independentemente do dia
agendado para sua apresentação.

PRINCIPAIS ASPECTOS CONSIDERADOS NA AVALIAÇÃO DO COMPILADOR

(1) Estrutura sintática:
(A) Introdução e detecção de erros léxicos e sintáticos;
(B) Como o programa/estrutura pode ser derivado das regras sintáticas?
(C) Gerar fontes que incluam instruções que originam conflito shift-reduce. Ações corretas?

(2) Variáveis:
(A) Variáveis omitidas - detectar erros;
(B) Variáveis duplicadas no mesmo escopo - detectar erros;
(C) Variáveis duplicadas em escopos distintos e não-aninhados - testar acessos;
(D) Variáveis duplicadas em escopos distintos e aninhados - testar acessos.

(3) Expressões:
(A) Identificar e testar coerções permitidas. Mostrar operações para compatibilização de tipos.
(B) Misturar tipos incompatíveis e testar detecção de erros.

(4) Passagem de parâmetros:
(A) Corretamente transmitidos ao subprograma chamado?
(B) Alterados/inalterados corretamente após retorno do subprograma chamado?

A linguagem a ser usada na geração de código é uma restrição da linguagem C, na qual
podem ser usados apenas os seguintes elementos da linguagem:
• Expressões aritméticas, lógicas e chamadas de funções.
• Todos os tipos de dados da linguagem.
• Todas as declarações da linguagem.
• Apenas os seguintes comandos de C:

- Abertura e fechamento de blocos;
- Sequenciamento de comandos;
- Atribuição;
- Chamadas de funções;
- Rótulos (labels) e comando goto;
- Comandos return, break e exit;
- Comandos de seleção APENAS da forma:

```
 if( condição ) goto l; 
```

```switch ( expressão ) {
case valor : { … }
}
```
• Nenhum outro comando da linguagem deve ser usado, como comandos de
iteração ou de seleção estruturados.

A linguagem deve resolver todos os problemas abaixo sem falhas:

## Problema 1:
Crie um programa que, dados três valores numéricos x, y e c, onde x e y são números racionais e c é um número inteiro, previamente armazenados no código fonte, avalia a expressão x 2 − y + c e imprime seu resultado na tela.

## Problema 2:
Crie um programa que leia uma quantidade desconhecida de números e informe quantos deles estão nos seguintes intervalos fechados: [0, 25], [26, 50], [51, 75] e [76, 100]. A entrada de dados deve terminar quando for lido um número negativo.

## Problema 3:
Crie um programa que leia duas matrizes numéricas e, quando possível, imprima a soma e o produto dessas matrizes. Caso uma operação não possa ser realizada para as matrizes lidas, imprima uma mensagem informando da impossibilidade.

## Problema 4:
Defina o tipo rational_t para representar números racionais. O tipo rational_t deve ser representado como um registro (ou tipo correspondente) com campos inteiros numerador e denominador. Em seguida, escreva os seguintes subprogramas:

A) Subprograma que, dados dois parâmetros inteiros a e b, onde b 6= 0, retorna um valor rational_t para representar a fração a/b.

B) Subprograma que, dados dois parâmetros do tipo rational_t, retorna true  se eles representam o mesmo número racional ou f alse, em caso contrário.

C) Subprogramas que retornem um valor rational_t correspondente a soma,  negação, subtração, multiplicação, inverso e divisão entre valores rational_t, passados como parâmetros (um subprograma por operação).

No programa principal, invoque cada um dos subprogramas e imprima os resultados produzidos, indicando numerador e denominador.

## Problema 5:
Crie um subprograma chamado mdc, com três argumentos n, m (passados por valor) e r (passado por referência), nesta ordem. O subprograma mdc deve calcular o maior divisor comum entre dois números naturais estritamente positivos n e m, de acordo com o seguinte algoritmo recursivo:
- Se n for um divisor de m, n é o maior divisor comum de n e m.
- Se m for um divisor de n, m é o maior divisor comum de n e m.
- Se n não for um divisor de m, e se m for maior que n, então o maior divisor comum de m e n é também o maior divisor comum de n e do resto da divisão de m por n.

O subprograma deve retornar seu resultado por meio de parâmetro r, que deve
ser posteriormente impresso na tela pelo programa principal.

## Problema 6:
Uma árvore binária de busca generaliza a ideia de listas encadeadas crescentes. Em uma árvore binária de busca, os nós têm um campo chave de um tipo ordenável e apresentam as seguintes propriedades: para qualquer nó n, a chave de n é maior ou igual à chave de qualquer nó na subárvore esquerda de n e menor ou igual à chave de qualquer nó na subárvore direita de n. Implemente uma árvore binária de busca com chaves de tipo inteiro e as seguintes operações:

A) Transforme uma sequência de valores em uma árvore binária de busca.

B) Encontre a chave mínima da árvore, indicando seu nível.

C) Encontre a chave máxima da árvore, indicando seu nível.

D) Imprima a árvore de busca na saída padrão, nível a nível.