# FPI - project 1

## Objetivos

a) Ler e gravar arquivos de imagens; OK
b) Exibir o conteúdo de um arquivo de imagem; OK 
c) Converter uma imagem colorida em uma imagem em tons de cinza;
d) Aplicar um esquema simples para quantização de imagens;
e) Utilizar uma biblioteca para gerenciar serviços de janela.

## Etapas

Parte I – Leitura e Gravação de Arquivos de Imagens (20 pontos) OK
Parte II – Leitura, Exibição e Operações sobre Imagens (80 pontos)

## Avaliação da parte II

a) (25 pontos) Espelhamento horizontal e vertical da imagem original. OK
    utilizando memcpy

b) (20 pontos) Conversão de imagem colorida para tons de cinza (luminância) OK
    L = 0.299*R + 0.587*G + 0.114*B,

        Pergunta: o que acontecerá com uma imagem em tons de cinza (Ri = Gi = Bi = Li) caso o cálculo
    de luminância seja aplicado a ela?

c) (25 pontos) Quantização (de tons) sobre as imagens em tons de cinza OK
        
        Para realização desta tarefa, sejam t1 e t2, respectivamente, os tons (presentes na
    imagem) com o menor e o maior valor de intensidade (i.e., luminância), e seja tam_int
    = (t2 - t1 + 1). Seja n o número de tons a ser utilizado no processo de quantização. Caso
    n  tam_int, nenhuma quantização é necessária (há tons suficientes para representar a
    imagem com seus tons originais). 
    
        No caso em que n < tam_int, o tamanho de cada bin é dado por tb = tam_int/n. 
    Assim, o 1o bin corresponderá ao intervalo [t1-0.5, t1-0.5+tb),
    o segundo bin ao intervalo [t1-0.5+tb, t1-0.5+2tb), e assim sucessivamente. Para o
    processo de quantização, dado um tom t_orig de um pixel da imagem original, utilize
    como seu tom quantizado o valor inteiro mais próximo do centro do intervalo do bin que
    contém t_orig.

d) (10 points) Salvamento da imagem resultante das operações realizadas em um arquivo  OK