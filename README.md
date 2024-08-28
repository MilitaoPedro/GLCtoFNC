# GLC -> Forma normal de Chomsky
Aplicativo desenvolvido em C++, que recebe como entrada um arquivo texto com uma GLC (Gramática Livre de Contexto) G’ e provém como saída um arquivo texto com a GLC G” na FNC (Forma normal de Chomsky) equivalente à G’.

## Padrões utilizados para as produções
– Variáveis: [A-Z]
- Variáveis criadas pelo algoritmo: 
  - X (x sendo o terminal da linguagem que a variável criada gera)
  - T<sub>n<sub> (∀n ∈ N}) (variáveis criadas na etapa final da passagem para Chomsky)
– Terminais: [a-z]
– Operador de definição: ->
– Separador de regras: | (ou quebra de linha)
– Lambda: .

## Chamada da função
A chamada é feita por linha de comando:
```> ./glc2fnc glc1.txt glc1fnc.txt```

## Exemplo de entrada e saída
### Exemplo de entrada (Texto dentro do arquivo de entrada)
```
S -> aS | bS | C | D
C -> c | .
D -> abc
D -> .
```
### Exemplo de saida (Texto dentro do arquivo de saida)
```
S' -> . | a | A'S | A'T1 | b | B'S | c
S -> a | A'S | A'T1 | b | B'S | c
A' -> a
B' -> b
C' -> c
T1 -> B'C'
```

