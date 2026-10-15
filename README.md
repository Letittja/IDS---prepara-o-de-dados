# IDS — preparação de dados e detecção de anomalias

Projeto em C para preparar capturas de tráfego de rede e identificar desvios no volume de pacotes. Os dados são agrupados em intervalos de 60 segundos, analisados por uma média móvel e apresentados em um gráfico com Python.

## Estrutura

| Arquivo | Função |
| --- | --- |
| `leitor.c` | Lê cinco capturas offline com libpcap e gera a série de pacotes por minuto. |
| `detector.c` | Calcula previsões, sinaliza anomalias e exibe NMSE e MAPE. |
| `gerador_graficos.py` | Compara tráfego real e previsto e destaca as anomalias. |
| `informacoes.md` | Reúne anotações de estudo, referências e observações do desenvolvimento. |

## Dados de entrada

Conforme as anotações do projeto, são utilizadas as capturas **inside tcpdump data** dos cinco dias da semana 2 de treinamento do dataset DARPA 1999. O endereço de origem está registrado em [informacoes.md](informacoes.md).

Extraia as capturas e organize-as na raiz do projeto nesta ordem (segunda a sexta-feira):

```text
inside.tcpdump/inside.tcpdump
inside.tcpdump (1)/inside.tcpdump
inside.tcpdump (2)/inside.tcpdump
inside.tcpdump (3)/inside.tcpdump
inside.tcpdump (4)/inside.tcpdump
```

Os caminhos estão definidos no vetor `pcap_files` de `leitor.c`. Caso os arquivos estejam em outro local, ajuste esse vetor e preserve a ordem cronológica. Cada caminho deve apontar para o arquivo de captura dentro da pasta.

As capturas, o SDK local, os executáveis e as saídas geradas são ignorados pelo Git e precisam ser obtidos ou produzidos localmente.

## Preparação do ambiente

Os comandos abaixo são para **Linux ou Ubuntu no WSL**, executados a partir da raiz do projeto. São necessários GCC, libpcap, Python 3, pandas e Matplotlib.

```bash
sudo apt update
sudo apt install build-essential libpcap-dev python3 python3-venv
python3 -m venv .venv
source .venv/bin/activate
python -m pip install pandas matplotlib
```

No WSL, manter o projeto e as capturas no sistema de arquivos Linux pode reduzir o tempo de leitura em comparação com o acesso pelo caminho `/mnt/c/`.

As anotações também registram o uso do Npcap no Windows. A pasta `npcap-sdk-1.16` é uma dependência local; os comandos de compilação deste guia utilizam a libpcap do Linux/WSL.

## Compilação e execução

Compile os dois programas:

```bash
gcc -Wall -Wextra leitor.c -o leitor -lpcap
gcc -Wall -Wextra detector.c -o detector -lm
```

Execute as etapas em sequência, com o ambiente Python ativado:

```bash
./leitor
./detector
python gerador_graficos.py
```

1. O leitor gera `pacotes_por_minuto.csv`.
2. O detector lê esse CSV, exibe as métricas no terminal e gera `resultados_predicao.csv`.
3. O script Python lê as previsões e salva `grafico_resultado.png` com resolução de 300 dpi.

Cada execução sobrescreve a saída da respectiva etapa. A leitura das capturas pode demorar devido ao volume de dados.

## Formato das saídas

`pacotes_por_minuto.csv`:

```csv
timestamp,total_pacotes,pacotes_por_segundo
920898000,267,4.45
```

O `timestamp` representa o início do intervalo em segundos Unix; a taxa média é calculada dividindo o total de pacotes por 60.

`resultados_predicao.csv`:

```csv
timestamp,real_pps,previsto_pps,erro_absoluto,is_anomalia
920898600,3.43,6.66,3.23,0
```

As taxas e o erro absoluto são expressos em pacotes por segundo (pps). O campo `is_anomalia` vale `1` quando o ponto excede o limite de detecção e `0` nos demais casos. O gráfico usa o índice das linhas no eixo horizontal.

## Modelo e parâmetros

O detector implementa uma **média móvel simples com janela fixa**. Para cada ponto, calcula a previsão usando os valores dos dez intervalos anteriores. Um ponto é marcado como anomalia quando:

```text
|real_pps - previsto_pps| > K × desvio_padrao_da_janela
```

O desvio padrão local tem um piso de `0.1` pps. Os parâmetros ficam no início de `detector.c`; recompile após alterá-los:

| Constante | Valor atual | Significado |
| --- | --- | --- |
| `JANELA_TAMANHO` | `10` | Quantidade de pontos anteriores usados na previsão. |
| `SENSIBILIDADE_K` | `3.0` | Multiplicador do desvio padrão; valores maiores aumentam o limite. |
| `MAX_LINHAS` | `10000` | Limite de pontos carregados do CSV. |

Os primeiros dez pontos inicializam a janela e não recebem previsão. O código exige mais pontos que o tamanho da janela. PMA (Média Móvel de Poisson) e DyWiSA são mencionados nas anotações como referências de estudo, mas ainda não estão implementados.

## Métricas e limitações

- **NMSE:** erro quadrático médio das previsões dividido pela variância global da série carregada. Uma série de variância zero causa divisão por zero na implementação atual.
- **MAPE:** média dos erros percentuais absolutos, em porcentagem. Para valores reais iguais a zero, o código usa a média global como denominador; se ela também for zero, ocorre divisão por zero.
- **Anomalias:** representam desvios estatísticos do volume de tráfego. Confirmar ataques exige comparar os intervalos detectados com os rótulos do dataset; essa validação não está implementada.

Na implementação atual do leitor, intervalos sem pacotes não são gravados, o primeiro pacote da série não entra na contagem e o último intervalo pendente não é salvo ao encerrar. Por isso, as linhas podem ter lacunas temporais e a janela de dez pontos pode abranger mais de dez minutos corridos. Esses comportamentos devem ser considerados antes de interpretar resultados ou comparar métricas.

## Problemas comuns

- **`pcap/pcap.h` não encontrado:** instale `libpcap-dev` no ambiente Linux/WSL em que está compilando.
- **`Is a directory` ao abrir uma captura:** confira se o caminho aponta para o arquivo interno, e não para a pasta.
- **Erro ao abrir `pacotes_por_minuto.csv`:** execute primeiro o leitor na raiz do projeto e confira as mensagens de abertura das capturas. Ele pula arquivos que não consegue abrir, o que pode produzir uma série incompleta.
- **Dados insuficientes no detector:** verifique se o CSV contém mais de dez pontos válidos.
- **Módulo Python ausente:** ative `.venv` e instale `pandas` e `matplotlib` nesse ambiente.
