#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LINHAS 10000 // Suporta até ~7 dias de minutos (5 dias = ~7200 minutos)
#define JANELA_TAMANHO 10 // Tamanho da janela deslizante (W = 10 minutos)
#define SENSIBILIDADE_K 3.0 // Multiplicador do desvio padrão para limite de anomalia

typedef struct {
    long timestamp;
    long total_pacotes;
    double pps; // Pacotes por segundo real
} PontoTráfego;

int main() {

    //Lê o arquivo CSV gerado na etapa anterior
    FILE *arquivo_in = fopen("pacotes_por_minuto.csv", "r");
    if (arquivo_in == NULL) {
        fprintf(stderr, "Erro ao abrir 'pacotes_por_minuto.csv'. Verifique se o pré-processamento foi executado.\n");
        return 1;
    }

    PontoTráfego dados[MAX_LINHAS];
    char cabecalho[256];
    int total_pontos = 0;

    // Ignorar a primeira linha (cabeçalho)
    fgets(cabecalho, sizeof(cabecalho), arquivo_in);

    // LEITURA DA SÉRIE TEMPORAL
    while (fscanf(arquivo_in, "%ld,%ld,%lf", 
                  &dados[total_pontos].timestamp, 
                  &dados[total_pontos].total_pacotes, 
                  &dados[total_pontos].pps) == 3) {
        total_pontos++;
        if (total_pontos >= MAX_LINHAS) break;
    }
    fclose(arquivo_in);

    printf("Total de pontos de 60s carregados: %d\n", total_pontos);

    if (total_pontos <= JANELA_TAMANHO) {
        fprintf(stderr, "Dados insuficientes para a janela deslizante.\n");
        return 1;
    }

    // CÁLCULO DA VARIÂNCIA / DESVIO PADRÃO GLOBAL DO TRÁFEGO REAL (para o NMSE)
    double soma_pps = 0.0;
    for (int i = 0; i < total_pontos; i++) {
        soma_pps += dados[i].pps;
    }
    double media_global = soma_pps / total_pontos;

    double soma_quad_diff_global = 0.0;
    for (int i = 0; i < total_pontos; i++) {
        soma_quad_diff_global += pow(dados[i].pps - media_global, 2);
    }
    double variancia_global = soma_quad_diff_global / total_pontos;

    // Abrir arquivo de saída para salvar previsões e anomalias
    FILE *arquivo_out = fopen("resultados_predicao.csv", "w");
    fprintf(arquivo_out, "timestamp,real_pps,previsto_pps,erro_absoluto,is_anomalia\n");

    // Variáveis estatísticas
    double soma_se = 0.0;     // Sum of Squared Errors
    double soma_ape = 0.0;    // Absolute Percentage Error
    int qtd_predicoes = 0;
    int qtd_anomalias = 0;

    // EXECUÇÃO DO MODELO PREDITIVO COM JANELA DESLIZANTE
    for (int i = JANELA_TAMANHO; i < total_pontos; i++) {
        
        // A) Previsão baseada nos W pontos anteriores (Média Móvel)
        double soma_janela = 0.0;
        for (int j = i - JANELA_TAMANHO; j < i; j++) {
            soma_janela += dados[j].pps;
        }
        double previsto_pps = soma_janela / JANELA_TAMANHO;
        double real_pps = dados[i].pps;

        // B) Cálculo do Erro e Desvio Padrão na Janela (para threshold dinâmico)
        double erro = real_pps - previsto_pps;
        double erro_abs = fabs(erro);

        double soma_quad_janela = 0.0;
        for (int j = i - JANELA_TAMANHO; j < i; j++) {
            soma_quad_janela += pow(dados[j].pps - (soma_janela / JANELA_TAMANHO), 2);
        }
        double desvio_padrao_janela = sqrt(soma_quad_janela / JANELA_TAMANHO);

        // Evitar divisão por zero no desvio padrão
        if (desvio_padrao_janela < 0.1) desvio_padrao_janela = 0.1;

        // C) Critério de Anomalia: desvio excede K * desvio_padrão_local
        int is_anomalia = 0;
        if (erro_abs > (SENSIBILIDADE_K * desvio_padrao_janela)) {
            is_anomalia = 1;
            qtd_anomalias++;
        }

        // D) Acumulação para NMSE e MAPE
        soma_se += pow(erro, 2);
        if (real_pps > 0) {
            soma_ape += (erro_abs / real_pps);
        } else {
            soma_ape += (erro_abs / media_global);
        }
        qtd_predicoes++;

        // E) Gravar no CSV de saída
        fprintf(arquivo_out, "%ld,%.2f,%.2f,%.2f,%d\n", 
                dados[i].timestamp, real_pps, previsto_pps, erro_abs, is_anomalia);
    }

    fclose(arquivo_out);

    // CÁLCULO FINAL DAS MÉTRICAS ESTATÍSTICAS (NMSE e MAPE)
    double mse = soma_se / qtd_predicoes;
    double nmse = mse / variancia_global;
    double mape = (soma_ape / qtd_predicoes) * 100.0;

    // EXIBIÇÃO DOS RESULTADOS ESTATÍSTICOS
    printf("\n----------------- RESULTADOS ESTATÍSTICOS -----------------\n");
    printf("Total de Minutos Analisados:%d\n", qtd_predicoes);
    printf("Tamanho da Janela Deslizante:%d minutos\n", JANELA_TAMANHO);
    printf("Anomalias/Ataques Detetados:%d\n", qtd_anomalias);
    printf("---------------------------------------------------------\n");
    printf("NMSE (Normalized Mean Square Error):%.4f\n", nmse);
    printf("MAPE (Mean Absolute Percent Error):%.2f%%\n", mape);
    printf("---------------------------------------------------------\n");
    printf("Resultados detalhados salvos em 'resultados_predicao.csv'.\n\n");

    return 0;
}