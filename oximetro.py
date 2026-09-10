import numpy as np
import matplotlib.pyplot as plt

def simular_oximetro(duracao=10, fc=75, spo2_real=97):
    """
    Simula o sinal de um oxímetro de pulso (PPG)
    fc = frequência cardíaca (bpm)
    spo2_real = saturação de oxigênio real (%)
    """
    t = np.linspace(0, duracao, 1000)
    freq_hz = fc / 60  # converte bpm para Hz

    # Sinal PPG: componente pulsátil (AC) + base constante (DC)
    vermelho = 1.0 + 0.05 * np.sin(2 * np.pi * freq_hz * t)
    infravermelho = 1.0 + 0.03 * np.sin(2 * np.pi * freq_hz * t)

    # Razão R (usada na fórmula real dos oxímetros)
    AC_vermelho = np.max(vermelho) - np.min(vermelho)
    DC_vermelho = np.mean(vermelho)
    AC_infra = np.max(infravermelho) - np.min(infravermelho)
    DC_infra = np.mean(infravermelho)

    R = (AC_vermelho / DC_vermelho) / (AC_infra / DC_infra)

    # Fórmula empírica aproximada (usada em oxímetros reais)
    spo2_calculado = 110 - 25 * R

    print(f"Frequência cardíaca: {fc} bpm")
    print(f"Razão R: {R:.3f}")
    print(f"SpO2 calculado: {spo2_calculado:.1f}%")

    plt.figure(figsize=(10, 4))
    plt.plot(t, vermelho, label="Luz Vermelha", color="red")
    plt.plot(t, infravermelho, label="Luz Infravermelha", color="darkred", alpha=0.6)
    plt.title("Sinal PPG simulado (Oxímetro de Pulso)")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Absorção de luz")
    plt.legend()
    plt.grid(True)
    plt.show()

    return spo2_calculado

# Exemplo de uso
simular_oximetro(fc=75, spo2_real=97)
