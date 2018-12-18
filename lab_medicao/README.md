# Experimento de Medição

## Introdução

Esse experimento tem como principal objetivo avaliar o desempenho de tempo entre a leitura e escrita entre processos distintos utilizando memória compartilhada e array, bem como comparar com envio de sinais.

## Metodologia

Foi utilizado o método ``clock_gettime`` da biblioteca time.h com a flag **CLOCK_MONOTONIC_COARSE** para capturar os instantes de tempo, foi utilizado a flag MONOTONIC por ser um contador que não sofre interferencia do usuário e COARSE para termos mais precisão, já que se não utilizado o tempo do SO mudar de modo usuário para modo kernel custaria precisão no tempo calculado.

O programa ``shm.c`` cria uma memória compartilhada e um array e executa a escrita de 1B (char 'a') em toda seu espaço e em seguinda percorre novamente o array verificando se foi de fato foi feito a escrita (essa segunda etapa entendemos que é o tempo de leitura). A execução da escrita/leitura da memória compartilhada e o array ocorrem de forma concorrente utilizando ``fork``

O programa ``signal-get-times.c`` cria dois processos em que um envia sinais e o outro recebe (análogo a arquitetura cliente-servidor). Como só é possivel enviar um inteiro por vez, foi assumido que **um envio de sinal é equivalente a leitura e escrita de 1B em uma memória compartilhada/array**

Para formartar as saidas e otimizar a geração dos dados, foi utilizado o script ``run.sh``, ele executa de forma paralela os programas de memoria compartilha e sinais.

Foi gerados 1000 amostras desse processo para quatro tamanho de memória distintos: 1MB, 128MB, 256MB e 512MB.

## Resultados

As saidas dos programas foram colocadas nessa [planilha](https://docs.google.com/spreadsheets/d/14-sDDpeBVOQDcqk5ZLbbrB74eAsFX99EiQgAd1GUcg4/edit?usp=sharing) para que fosse gerado os gráficos e possamos fazer as análises.

Como já era esperado, a utilização de sinais como forma de comunicação entre processo para leitura e escrita de dados é bastante custosa, ao plotar os gráficos com a média de tempo obtido pelas 1000 amostras de sinais fica praticamente impossivel de verificar os dados de memororia compartilhada e array de tão descrepante que é a diferença.

Para se ter uma ideia, nem o tempo de leitura e escrita de uma memoria compartilhada de 512MB é superior ao envio de 1MB de sinais de um processo para outro, desta forma, podemos afirmar que o uso de sinais deve ser mais focado em flags, a vantagem de não compartilhar região da memória, logo região crítica, é insignificante ao comparar o seu custo de tempo.

![grafico1](./assets/barras-com-sinal.png)


![grafico2](./assets/reta-com-sinal.png)

Desta forma, para analisar melhor os resultados, plotamos os dados sem as informações coletadas de sinais. Podemos notar com os gráficos abaixo muita similaridade ao comparar o tempo de leitura e escrita de cada categoria. Como também era esperado, verificamos que o tempo de ler e escrever na memória compartilhada é maior que no array, já que a memoria compartilhada fica separado do processo, diferente do array que é alocado na Heap.


![grafico3](./assets/barras-sem-sinal.png)

![grafico4](./assets/retas-sem-sinal.png)
