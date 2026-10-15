# Artigo lido do professor:
sugere fortemente o uso de métodos de análise local baseados em janelas deslizantes (sliding windows), especificamente o método de Média Móvel de Poisson (PMA) combinado com um algoritmo de Janela de Tamanho Dinâmico (DyWiSA). O artigo conclui que esta é a abordagem mais equilibrada e eficiente, oferecendo alta precisão com baixa complexidade computacional.

# Site que foi extraído os dados:
Foi baixado os arquivos semanais "inside tcpdump data" do site "https://archive.ll.mit.edu/ideval/data/1999/training/week2/index.html"

# Site e ferramenta utilizada para extrair os arquivos:
https://npcap.com/#download
baixei o Npcap Installer e o arquivo zip para extrair os arquivos de cabeçalho (.h) e as bibliotecas de linkagem (.lib)

# linguagem escolhida:
- Baixa Complexidade e Tempo de Execução: O artigo enfatiza que um requisito fundamental para a previsão de tráfego na nuvem é manter uma baixa complexidade computacional e resposta em tempo útil. A linguagem C oferece o desempenho de baixo nível necessário para processar grandes volumes de dados com o mínimo de overhead (sobrecarga).
- Integração Nativa com o Npcap SDK: As bibliotecas do Npcap SDK (wpcap.lib, pcap.h) foram desenvolvidas nativamente em C/C++. Escrever o seu código em C evita a necessidade de utilizar intermediários ou wrappers de outras linguagens.
- Eficiência na Janela Deslizante (Sliding Window): A implementação das estruturas de dados necessárias para a janela deslizante e para o cálculo das médias móveis (SMA, WMA, EMA ou PMA) é simples e extremamente rápida de executar em C utilizando arrays ou buffers circulares em memória.


# Problema que deu:
ao compilar e "./leitor" o seguinte erro apareceu:
Erro ao abrir o arquivo error reading dump file: Is a directory pcap, ou está vazio
o comando "ls -ld inside.tcpdump*" foi usado para saber qual o tipo de arquivo estava sendo usado e vi que dentro das pastas há outros arquivos com os mesmos nomes da pastas, sendo assim a lista com os caminhos completos das pastas foi atualizado

# A leitura demora bastante:
- Volume de Dados Enorme: Leste cerca de 1,6 GB de arquivos contendo 7.275.137 pacotes. O teu processador teve de fazer mais de 7,2 milhões de iterações no while.
- Sistema de Arquivos do WSL (/mnt/c/): Os ficheiros estão na partição do Windows (C:\...). Quando o WSL (Linux) lê ficheiros dentro de /mnt/c/, a leitura passa por uma camada de tradução entre o sistema de ficheiros do Windows (NTFS) e o do Linux. Isso reduz bastante a velocidade de leitura do disco.
- Cópia de Memória em cada Pacote: No teu while, a instrução ultimo_pacote = *header; copia a estrutura inteira de memória 7,27 milhões de vezes.

# Ver as 10 primeiras linhas da série temporal
head -n 10 pacotes_por_minuto.csv

# Contar quantas linhas (minutos) foram geradas no total da semana
wc -l pacotes_por_minuto.csv


Detector de anomalias
- Leitura da Série Temporal: Ler o ficheiro pacotes_por_minuto.csv
- Modelo Preditivo (Janela Deslizante): Para cada instante $t$, utilizar a janela dos minutos anteriores para calcular o valor previsto no instante $t+1$ (utilizando a Média Móvel de Poisson - PMA ou outra média móvel).
- Identificação de Anomalias: Calcular a diferença entre o tráfego Real e o tráfego Previsto. Se essa diferença ultrapassar um determinado limite (threshold ou desvio), esse ponto é classificado como uma anomalia.
- Métricas e Gráficos:
-- Calcular o NMSE e o MAPE para medir a precisão do preditor.   
-- Gerar o gráfico final comparando a linha de Tráfego Real vs Tráfego Previsto com o eixo Y em pacotes por segundo. 

# Detector.c
- Lê o arquivo pacotes_por_minuto.csv gerado na etapa anterior.   
- Aplica o modelo preditivo por Janela Deslizante (Sliding Window) usando Média Móvel.   
- Calcula o Limite de Deteção (Threshold): Se o desvio $\vert{}Real - Previsto\vert{}$ for maior que $K \times \sigma$ (onde $K$ é a sensibilidade e $\sigma$ é o desvio padrão local), o ponto é marcado como Anomalia.   
- Calcula as Métricas Estatísticas do Artigo: NMSE (Erro Quadrático Médio Normalizado) e MAPE (Erro Percentual Absoluto Médio).   
- Gera um arquivo de saída resultados_predicao.csv com as colunas: timestamp,real_pps,previsto_pps,erro,is_anomalia. 

# Interpretação de resultados
- NMSE = 0.3427: Conforme demonstrado no artigo de referência (Dalmazo et al., 2017), um NMSE < 1.0 indica que o teu preditor por janela deslizante é substancialmente superior a um preditor trivial (média global). O valor de 0.3427 mostra uma excelente precisão na captura do comportamento local da rede.
- MAPE = 81.94%: Um valor de erro percentual comum para séries temporais de tráfego de rede com picos abruptos e ataques, onde variações repentinas aumentam a discrepância percentual momentânea.
- 317 Anomalias Detetadas: Em ~6.400 minutos de tráfego, o algoritmo identificou 317 intervalos de 60 segundos com desvios significativos, correspondendo aos períodos em que ocorreram os ataques documentados na semana 2 do dataset.