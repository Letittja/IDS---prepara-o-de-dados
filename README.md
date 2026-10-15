Site que foi extraído os dados:
Foi baixado os arquivos semanais "inside tcpdump data" do site "https://archive.ll.mit.edu/ideval/data/1999/training/week2/index.html"

Site e ferramenta utilizada para extrair os arquivos:
https://npcap.com/#download
baixei o Npcap Installer e o arquivo zip para extrair os arquivos de cabeçalho (.h) e as bibliotecas de linkagem (.lib)

linguagem escolhida:
-Baixa Complexidade e Tempo de Execução: O artigo enfatiza que um requisito fundamental para a previsão de tráfego na nuvem é manter uma baixa complexidade computacional e resposta em tempo útil. A linguagem C oferece o desempenho de baixo nível necessário para processar grandes volumes de dados com o mínimo de overhead (sobrecarga).
-Integração Nativa com o Npcap SDK: As bibliotecas do Npcap SDK (wpcap.lib, pcap.h) foram desenvolvidas nativamente em C/C++. Escrever o seu código em C evita a necessidade de utilizar intermediários ou wrappers de outras linguagens.
-Eficiência na Janela Deslizante (Sliding Window): A implementação das estruturas de dados necessárias para a janela deslizante e para o cálculo das médias móveis (SMA, WMA, EMA ou PMA) é simples e extremamente rápida de executar em C utilizando arrays ou buffers circulares em memória.