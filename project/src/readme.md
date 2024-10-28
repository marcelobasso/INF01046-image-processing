# FPI - project 1

## Part 1
### Objetivos

a) Ler e gravar arquivos de imagens;<br/>
b) Exibir o conteúdo de um arquivo de imagem;<br/>
c) Converter uma imagem colorida em uma imagem em tons de cinza;<br/>
d) Aplicar um esquema simples para quantização de imagens;<br/>
e) Utilizar uma biblioteca para gerenciar serviços de janela.<br/>

### Etapas

Parte I – Leitura e Gravação de Arquivos de Imagens (20 pontos)<br/>
Parte II – Leitura, Exibição e Operações sobre Imagens (80 pontos)

### Avaliação da parte II

a) (25 pontos) Espelhamento horizontal e vertical da imagem original.<br/>
b) (20 pontos) Conversão de imagem colorida para tons de cinza (luminância)<br/>
c) (25 pontos) Quantização (de tons) sobre as imagens em tons de cinza<br/>
d) (10 points) Salvamento da imagem resultante das operações realizadas em um arquivo<br/>


## Part 2
### Objetivos

a) Cálculo de histograma, transformações lineares sobre pixels (ajuste de brilho e
contraste e cálculo do negativo), equalização e “matching” de histograma;<br/>
b) Conceitos fundamentais como convolução e filtragem no domínio espacial. 

### Avaliação da parte I

1) (20 pontos) Calcular e exibir o histograma de uma imagem em tons de cinza<br/> 
    (8 bits por pixel). Caso a imagem informada como entrada seja colorida, converta-a para tons de cinza
    (luminância) e então calcule seu histograma. Exiba o histograma em uma janela separada,
    onde cada coluna da imagem representa um tom de cinza. Normalize a altura das colunas
    para obter uma representação apropriada.

2) (15 pontos) Ajustar o brilho de uma imagem (e exibí-la), somando ao valor de cada pixel
    um escalar no intervalo [-255, 255]. Certifique-se que o resultado da operação aplicado a
    cada pixel se encontra na faixa [0,255], ajustando-o para zero ou 255 quando necessário. No
    caso de imagens coloridas, aplique o algoritmo para cada um dos canais (R, G, B)
    independentemente.

3) (15 pontos) Ajustar o contraste de uma imagem (e exibí-la), multiplicando cada pixel por
    um escalar no intervalo (0, 255]. Certifique-se que o resultado da operação aplicado a cada
    pixel se encontra na faixa [0,255], ajustando-o para 255 quando necessário. No caso de
    imagens coloridas, aplique o algoritmo para cada um dos canais (R, G, B)
    independentemente. 

4) (10 pontos) Calcular e exibir o negativo de uma imagem, calculando o novo valor de cada
    pixel como: <novo valor> = 255 - <antigo valor>. No caso de imagens coloridas, aplique o
    algoritmo para cada um dos canais (R, G, B) independentemente.

5) (25 pontos) Equalizar o histograma de uma imagem, exibindo as imagens antes e depois
    da equalização. Para imagens em tons de cinza, exibir os histogramas antes e depois da
    equalização. No caso de imagens coloridas, para cada um dos canais (R, G, B), utilize o
    histograma cumulativo obtido a partir da imagem de luminância. 

6) (15 pontos) Realizar Histogram Matching de pares de imagens em tons de cinza. 


### Avaliacao da parte II

7) (25 pontos) Reduzir uma imagem (zoom out) utilizando fatores de redução sx e sy (sx e sy
     1), onde sx não é necessariamente igual a sy. Para tanto, defina um retângulo com
    dimensões sy e sx, e mova-o sobre a imagem, de modo que os retângulos nunca se
    sobreponham e que nenhum pixel da imagem deixe de ser coberto. Para cada posição do
    retângulo, calcule as médias (R, G e B) dos pixels sob o retângulo, utilizando estes resultados
    na imagem de saída. Caso o retângulo vá além das dimensões da imagem original, calcule as
    médias usando apenas os pixels sob o retângulo.

8) (25 pontos) Ampliar a imagem (zoom in) utilizando um fator de 2x2 a cada vez. Visto que
    a imagem será 4 vezes maior que a original, certifique-se de que você alocará memória para
    tal, apropriadamente. Implemente o processo de ampliação utilizando operações 1-D em dois
    passos. Primeiro, insira uma linha e uma coluna em branco entre cada duas linhas e colunas
    originais, respectivamente (veja ilustração na figura a seguir). Após, linearmente interpole os
    valores para preencher os espaços ao longo de todas as linhas (ou colunas) e, finalmente, ao
    longo de todas as colunas (ou linhas).

9) (15 pontos) Rotacionar imagem de 90o
    (tanto no sentido horário como no sentido antihorário). A operação de rotação deve poder ser aplicada múltiplas vezes de modo a permitir
    obter rotações de +/- 180o
    , +/- 270o
    , etc.
    
10) (35 pontos) Implementar um procedimento para realizar convolução entre uma imagem
    e um filtro 3x3 arbitrário. De modo a simplificar sua impementação, ignore as bordas da
    imagem original e aplique a convolução apenas no seu interior. Exceto para o filtro passa
    baixas, aplique a convolução apenas a imagens de luminância. A figura a seguir ilustra a
    aplicação de uma operação de convolução ao pixel E utilizando um kernel 3x3. Note que o
    kernel é rotacionado de 180o
    possibilidade de utilização de pesos negativos, o resultado da convolução pode ser maior que
    255 ou menor que zero. Na sua implementação, você deverá aproximar estes casos com os
    valores 255 e zero, respectivamente, para os casos (i) a (iii) abaixo. Para os demais, some
    127 ao resultado da convolução antes de fazer esta aproximação (clampping). A interface do
    seu programa deverá aceitar valores arbitrários para os pesos dos filtros. Teste o seu
    procedimento com os kernels (i) a (vii) a seguir. 