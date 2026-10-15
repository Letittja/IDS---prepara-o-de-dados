import pandas as pd
import matplotlib.pyplot as plt

# Carregar os resultados gerados pelo programa em C
df = pd.read_csv('resultados_predicao.csv')

plt.figure(figsize=(14, 6))

# Plot do Tráfego Real e Previsto
plt.plot(df.index, df['real_pps'], label='Tráfego Real (pps)', color='dodgerblue', alpha=0.7, linewidth=1)
plt.plot(df.index, df['previsto_pps'], label='Tráfego Previsto (pps)', color='orange', alpha=0.8, linewidth=1)

# Marcação das Anomalias
anomalias = df[df['is_anomalia'] == 1]
plt.scatter(anomalias.index, anomalias['real_pps'], color='red', label='Anomalia/Ataque', s=20, zorder=5)

plt.xlabel('Tempo (Minutos da Semana 2)')
plt.ylabel('Pacotes por Segundo (pps)')
plt.title('Deteção de Anomalias em Tráfego de Rede (Granularidade 60s)')
plt.legend(loc='upper right')
plt.grid(True, linestyle='--', alpha=0.5)

# Salva a imagem em alta resolução para colar no relatório
plt.tight_layout()
plt.savefig('grafico_resultado.png', dpi=300)
print("Gráfico 'grafico_resultado.png' gerado com sucesso!")