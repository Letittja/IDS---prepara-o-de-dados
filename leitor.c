#include <stdio.h>
#include <stdlib.h>
#include <pcap/pcap.h>

int main(){
    //Declarando variável de buffer para armazenar mensagens de erro
    char errbuf[PCAP_ERRBUF_SIZE];

    //Lista de arquivos pcap a serem lidos
    const char *pcap_files[] = {
        "inside.tcpdump/inside.tcpdump",
        "inside.tcpdump (1)/inside.tcpdump",
        "inside.tcpdump (2)/inside.tcpdump",
        "inside.tcpdump (3)/inside.tcpdump",
        "inside.tcpdump (4)/inside.tcpdump"
    };
    int total_arquivos = 5;

    //Lista de dias da semana
    const char *dias_semana[] = {
        "Segunda-feira",
        "Terça-feira",
        "Quarta-feira",
        "Quinta-feira",
        "Sexta-feira"
    };

    //Abrir arquivo CSV para agrupamento de pacotes por 60 segundos
    FILE *arquivo = fopen("pacotes_por_minuto.csv", "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo CSV para escrita.\n");
        return 1;
    }
    fprintf(arquivo, "timestamp,total_pacotes,pacotes_por_segundo\n");

    long inicio_intervalo = 0; //Variável para armazenar o início do intervalo de 60 segundos
    long pacotes_minuto = 0; //Variável para armazenar o número de pacotes lidos em 60 segundos
    long total_pacotes_semana = 0; //Variável para armazenar o total de pacotes lidos por semana

    //Abrir os arquivos pcap offline
    for (int i = 0; i < total_arquivos; i++) {

        pcap_t *handle = pcap_open_offline(pcap_files[i], errbuf);
        
        //Verificando se está vazio
        if (handle == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo %s: %s\n Ou está vazio\n", pcap_files[i], errbuf);
            continue; //Pular para o próximo arquivo pcap mesmo com a falha
        }else{
            printf("Lendo %s (%s)...\n", dias_semana[i], pcap_files[i]);
            printf("Arquivo pcap aberto com sucesso!\n");
        }

        //ler o arquvio pcap
        struct pcap_pkthdr *header; //Variável para armazenar o conteúdo extraído do arquivo pcap
        const u_char *packet; //Ponteiro para o pacote capturado
        int contador = 0; //Contador de pacotes lidos
        
        //Variáveis para armazenar o primeiro e último pacote lido
        struct pcap_pkthdr primeiro_pacote;
        struct pcap_pkthdr ultimo_pacote;
        
        //Loop para ler os pacotes e imprimir informações sobre eles por semana
        while(pcap_next_ex(handle, &header, &packet) >= 0) {
            contador++;
            total_pacotes_semana++;

            if (contador == 1) {
                primeiro_pacote = *header; //Armazena o primeiro pacote lido
            }else{
                ultimo_pacote = *header; //Armazena o último pacote lido
            }
            //lógica dos 60 segundos em tempo real
            long ts_pacote = (long)header->ts.tv_sec; //Timestamp do pacote atual
            
            //Inicia o intervalo de 60 segundos
            if (inicio_intervalo == 0) {
                inicio_intervalo = ts_pacote; 
            //Verifica se o pacote atual está dentro do intervalo de 60 segundos
            }else if (ts_pacote < inicio_intervalo + 60) {
                pacotes_minuto++;
            }else{
                //Acabou o intervalo, então ele grava na linha do CSV
                double pps = (double)pacotes_minuto / 60.0; //Calcula pacotes por segundo
                fprintf(arquivo, "%ld,%ld,%.2f\n", inicio_intervalo, pacotes_minuto, pps);

                //Reinicia o contador de pacotes e o início do intervalo
                while (ts_pacote >= inicio_intervalo + 60) {
                    inicio_intervalo += 60; //Avança o início do intervalo para o próximo minuto
                }
                pacotes_minuto = 1; //Reinicia o contador de pacotes para o novo intervalo
            }
        }
        //Resumo de pacotes lidos por dias da semana
        printf("%s(%s):\n %d Pacotes lidos\n", dias_semana[i], pcap_files[i], contador);
        printf("Primeiro pacote lido em %ld.%06ld segundos\n", (long)primeiro_pacote.ts.tv_sec, (long)primeiro_pacote.ts.tv_usec);
        printf("Último pacote lido em %ld.%06ld segundos\n", (long)ultimo_pacote.ts.tv_sec, (long)ultimo_pacote.ts.tv_usec);
        printf("-------------------------------\n");
        //Fechar o arquivo pcap
        pcap_close(handle);
    }    
    //Fechar o arquivo CSV
    fclose(arquivo); 
    return 0;

}