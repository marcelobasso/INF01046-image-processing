1) (20 pontos) Configure o seu ambiente de programação (e.g., Visual Studio, etc.) compile
e execute o programa básico disponibilizado no Apêndice A deste documento. Este
programa simples lhe permitirá capturar e exibir vídeos em tempo real, provendo a estrutura
sobre a qual você implementará as tarefas solicitadas.
Com cada quadro do vídeo capturado pela câmera, realize as operações abaixo e exiba o
frame resultante em uma janela ao lado da original. Para utilização dos comandos
mencionados para completar a tarefa, pode ser necessário algum tipo de pre-processamento
aplicado ao quadro em questão. Neste caso, é parte da tarefa a identificação e aplicação de tal
pre-processamento.

2) (20 pontos) Utilize o comando GaussianBlur para aplicar borramento ao video. Utilize um
Trackbar para definir o tamanho do kernel Gaussiano.

3) (20 pontos) Utilize o comando Canny para detectar as arestas no vídeo.

4) (20 pontos) Utilize o comando Sobel para obter uma estimativa do gradiente do vídeo. 

5) (20 pontos) Utilize o comando convertTo para realizar ajuste de brilho, ajuste de contraste, e
obter o negativo do video.

A esta altura, você já deve encontrar-se minimamente familiarizado com a documentação de
OpenCV. Para os ítens abaixo, identifique os comandos que serão necessários para a realização
das operações solicitadas e aplique-os ao vídeo, como feito nos ítens (2) a (5).

6) (10 pontos) Conversão de cores (RGB) para tons de cinza (grayscale).

7) (20 pontos) Redimensionamento do vídeo para a metade do número de píxels em cada
uma de suas dimensões.

8) (20 pontos) Rotação do vídeo de 90 graus.

9) (20 pontos) Espelhamento do vídeo (horizontal e vertical).

10) (30 pontos) Gravação de vídeo, levando em conta todos os efeitos acima, exceto Rotação
e Redimentsionamento, visto que estas operações alteram as dimensões originais do frame,
o que tenderia a ocasionar um erro durante a tentativa de gravação. 