## Universidade Federal do Rio Grande do Norte

## Departamento de Informática e Matemática Aplicada

## DIM0548 - Engenharia de Linguagens

## Problemas para Experimentação dos Compiladores

Este documento descreve os problemas que serão utilizados para a avaliação
dos compiladores do projeto da disciplina Engenharia de Linguagens. Para cada
problema, crie um programa que o solucione utilizando a linguagem proposta
pelo grupo, ou seja, aquela aceita pelo compilador criado pelo grupo. Esses
programas,ou versões com pequenas modificações, serão compilados para o sub-
conjunto da linguagem C definido na especificação do compilador (Problema 8)
e posteriormente executados sobre um conjunto de dados de entrada durante a
avaliação. As soluções apresentadas devem ser adaptadas àscaracterísticas de
cada linguagem, mas sempre atendendo às funcionalidades exigidas.

# Problema 1:

Crie um programa que, dados três valores numéricosx,yec, ondexeysão
números racionais ecé um número inteiro, previamente armazenados no código-
fonte, avalia a expressãox^2 −y+ce imprime seu resultado na tela.

# Problema 2:

Crie um programa que leia uma quantidade desconhecida de números e informe
quantos deles estão nos seguintes intervalos fechados: [0,25], [26, 50], [51, 75]
e [76, 100]. A entrada de dados deve terminar quando for lido um número
negativo.

# Problema 3:

Crie um programa que leia duas matrizes numéricas e, quando possível, imprima
a soma e o produto dessas matrizes. Caso uma operação não possa ser realizada
para as matrizes lidas, imprima uma mensagem informando da impossibilidade.

# Problema 4:

Defina o tiporational_tpara representar números racionais. O tiporational_t
deve ser representado como um registro (ou tipo correspondente) com campos
inteirosnumeradoredenominador. Em seguida, escreva os seguintes subpro-
gramas:

```
A) Subprograma que, dados dois parâmetros inteirosaeb, ondeb 6 = 0, retorna
um valorrational_tpara representar a fraçãoa/b.
```

### 1

```
B) Subprograma que, dados dois parâmetros do tiporational_t, retornatrue
se eles representam o mesmo número racional ouf alse, em caso contrário.
```

```
C) Subprogramas que retornem um valorrational_tcorrespondente a soma,
negação, subtração, multiplicação, inverso e divisão entre valoresrational_t,
passados como parâmetros (um subprograma por operação).
```

No programa principal, invoque cada um dos subprogramas e imprima os resul-
tados produzidos, indicando numerador e denominador.

# Problema 5:

Crie um subprograma chamadomdc, com três argumentosn,m(passados por
valor) er(passado por referência), nesta ordem. O subprogramamdcdeve cal-
cular o maior divisor comum entre dois números naturais estritamente positivos
nem, de acordo com o seguinte algoritmo recursivo:

- Senfor um divisor dem,né o maior divisor comum denem.
- Semfor um divisor den,mé o maior divisor comum denem.
- Sennão for um divisor dem, e semfor maior quen, então o maior
  divisor comum demené também o maior divisor comum dene do resto
  da divisão demporn.

O subprograma deve retornar seu resultado por meio de parâmetror, que deve
ser posteriormente impresso na tela pelo programa principal.

# Problema 6

### 1

# :

Uma árvore binária de busca generaliza a ideia de listas encadeadas crescentes.
Em uma árvore binária de busca, os nós têm um campo chave de um tipo
ordenável e apresentam as seguintes propriedades: para qualquer nón, a chave
dené maior ou igual à chave de qualquer nó na subárvore esquerda dene
menor ou igual à chave de qualquer nó na subárvore direita den. Implemente
uma árvore binária de busca com chaves de tipo inteiro e as seguintes operações:

```
A) Transforme uma sequência de valores em uma árvore bináriade busca.
```

```
B) Encontre a chave mínima da árvore, indicando seu nível.
```

```
C) Encontre a chave máxima da árvore, indicando seu nível.
```

```
D) Imprima a árvore de busca na saída padrão, nível a nível.
```
