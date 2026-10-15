#include <stdio.h>
#include <stdlib.h>
#include <pcap/pcap.h>

int main(){
    //Declarando variável de buffer para armazenar mensagens de erro
    char errbuf[PCAP_ERRBUF_SIZE];

    //Lista de arquivos pcap a serem lidos
    const char *pcap_files[] = {
        "inside.tcpdump",
        "inside.tcpdump (1)",
        "inside.tcpdump (2)",
        "inside.tcpdump (3)",
        "inside.tcpdump (4)"
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

    //Abrir os arquivos pcap offline
    for (int i = 0; i < total_arquivos; i++) {

        pcap_t *handle = pcap_open_offline(pcap_files[i], errbuf);
        
        //Verificando se está vazio
        if (handle == NULL) {
            fprintf(stderr, "Erro ao abrir o arquivo %s pcap, ou está vazio\n", errbuf);
            continue; //Pular para o próximo arquivo pcap mesmo com a falha
        }else{
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
            if (contador == 1) {
                primeiro_pacote = *header; //Armazena o primeiro pacote lido
            }else{
                ultimo_pacote = *header; //Armazena o último pacote lido
            }
        }
        //Resumo de pacotes lidos por dias da semana
        printf("-------------------------------\n");
        printf("%s(%s):\n %d Pacotes lidos\n", dias_semana[i], pcap_files[i], contador);
        printf("Primeiro pacote lido às %ld.%06ld\n", (long)primeiro_pacote.ts.tv_sec, (long)primeiro_pacote.ts.tv_usec);
        printf("Último pacote lido às %ld.%06ld\n", (long)ultimo_pacote.ts.tv_sec, (long)ultimo_pacote.ts.tv_usec);

        //Fechar o arquivo pcap
        pcap_close(handle);
    }
    return 0;
}
