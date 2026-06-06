import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, RadioButtons

class BiquadFilter:
    """Implémentation du filtre Biquad basée sur DadDSP::cBiQuad"""
    
    def __init__(self, filter_type='LPF', cutoff_freq=1000, gain_db=0, bandwidth=1.0, sample_rate=48000):
        self.sample_rate = sample_rate
        self.cutoff_freq = cutoff_freq
        self.gain_db = gain_db
        self.bandwidth = bandwidth
        self.filter_type = filter_type
        self.calculate_coefficients()
    
    def calculate_coefficients(self):
        """Calcul des coefficients selon les formules RBJ (Cookbook)"""
        kPi = np.pi
        kNaturalLog2 = np.log(2)
        
        # Variables intermédiaires
        A = 10 ** (self.gain_db / 40)
        omega = 2 * kPi * self.cutoff_freq / self.sample_rate
        sn = np.sin(omega)
        cs = np.cos(omega)
        alpha = sn * np.sinh(kNaturalLog2 / 2 * self.bandwidth * omega / sn)
        beta = np.sqrt(A + A)
        
        # Calcul des coefficients selon le type de filtre
        if self.filter_type == 'LPF' or self.filter_type == 'LPF24':
            # Low-pass filter
            b0 = (1 - cs) / 2
            b1 = 1 - cs
            b2 = (1 - cs) / 2
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
            
        elif self.filter_type == 'HPF' or self.filter_type == 'HPF24':
            # High-pass filter
            b0 = (1 + cs) / 2
            b1 = -(1 + cs)
            b2 = (1 + cs) / 2
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
            
        elif self.filter_type == 'BPF':
            # Band-pass filter
            b0 = alpha
            b1 = 0
            b2 = -alpha
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
            
        elif self.filter_type == 'NOTCH':
            # Notch filter
            b0 = 1
            b1 = -2 * cs
            b2 = 1
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
            
        elif self.filter_type == 'PEQ':
            # Peaking EQ filter
            b0 = 1 + (alpha * A)
            b1 = -2 * cs
            b2 = 1 - (alpha * A)
            a0 = 1 + (alpha / A)
            a1 = -2 * cs
            a2 = 1 - (alpha / A)
            
        elif self.filter_type == 'LSH':
            # Low shelf filter
            b0 = A * ((A + 1) - (A - 1) * cs + beta * sn)
            b1 = 2 * A * ((A - 1) - (A + 1) * cs)
            b2 = A * ((A + 1) - (A - 1) * cs - beta * sn)
            a0 = (A + 1) + (A - 1) * cs + beta * sn
            a1 = -2 * ((A - 1) + (A + 1) * cs)
            a2 = (A + 1) + (A - 1) * cs - beta * sn
            
        elif self.filter_type == 'HSH':
            # High shelf filter
            b0 = A * ((A + 1) + (A - 1) * cs + beta * sn)
            b1 = -2 * A * ((A - 1) + (A + 1) * cs)
            b2 = A * ((A + 1) + (A - 1) * cs - beta * sn)
            a0 = (A + 1) - (A - 1) * cs + beta * sn
            a1 = 2 * ((A - 1) - (A + 1) * cs)
            a2 = (A + 1) - (A - 1) * cs - beta * sn
            
        else:  # AFP (All-pass)
            b0 = 1 - alpha
            b1 = -2 * cs
            b2 = 1 + alpha
            a0 = 1 + alpha
            a1 = -2 * cs
            a2 = 1 - alpha
        
        # Normalisation des coefficients
        self.m_a0 = b0 / a0
        self.m_a1 = b1 / a0
        self.m_a2 = b2 / a0
        self.m_a3 = a1 / a0
        self.m_a4 = a2 / a0
    
    def gain_db_at_freq(self, freq):
        """Calcul du gain en dB à une fréquence donnée"""
        kPi = np.pi
        
        # Calcul de Phi
        Phi = 4 * np.sin((kPi * freq / self.sample_rate) ** 2)
        
        # Calcul du numérateur et dénominateur
        num = ((self.m_a0 * self.m_a2 * (Phi ** 2)) + 
               (self.m_a0 + self.m_a1 + self.m_a2) ** 2 - 
               (((self.m_a0 * self.m_a1) + (4 * self.m_a0 * self.m_a2) + 
                 (self.m_a1 * self.m_a2)) * Phi))
        
        denum = (self.m_a4 * (Phi ** 2)) + \
                (self.m_a3 + self.m_a4 + 1) ** 2 - \
                (((self.m_a3 * self.m_a4) + self.m_a3 + (4 * self.m_a4)) * Phi)
        
        # Protection division par zéro
        if denum == 0:
            denum = 1e-10
        
        # Calcul du gain en dB
        gain = 20 * np.log10(np.sqrt(num / denum))
        
        # Doubler le gain pour les filtres 24dB/oct
        if self.filter_type in ['LPF24', 'HPF24']:
            gain = gain * 2
        
        return gain
    
    def get_frequency_response(self, num_points=1000):
        """Calcul de la réponse en fréquence"""
        frequencies = np.logspace(np.log10(20), np.log10(self.sample_rate / 2), num_points)
        magnitude_db = np.array([self.gain_db_at_freq(f) for f in frequencies])
        
        # Calcul de la phase
        omega = 2 * np.pi * frequencies / self.sample_rate
        
        # Calcul de H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
        z = np.exp(1j * omega)
        numerator = self.m_a0 + self.m_a1 * z**(-1) + self.m_a2 * z**(-2)
        denominator = 1 + self.m_a3 * z**(-1) + self.m_a4 * z**(-2)
        H = numerator / denominator
        
        phase_deg = np.angle(H) * 180 / np.pi
        
        # Doubler la phase pour les filtres 24dB/oct
        if self.filter_type in ['LPF24', 'HPF24']:
            phase_deg = phase_deg * 2
        
        return frequencies, magnitude_db, phase_deg


# ========== Interface graphique ==========

# Création de la figure
fig = plt.figure(figsize=(14, 9))
gs = fig.add_gridspec(3, 2, height_ratios=[2, 2, 1], hspace=0.4, wspace=0.3)

ax_mag = fig.add_subplot(gs[0, :])
ax_phase = fig.add_subplot(gs[1, :])

# Paramètres initiaux
initial_freq = 1000
initial_bandwidth = 1.0
initial_gain = 0
fs = 48000

# Création du filtre initial
biquad = BiquadFilter('LPF', initial_freq, initial_gain, initial_bandwidth, fs)
w, mag, phase = biquad.get_frequency_response()

# Tracé du gain
line_mag, = ax_mag.semilogx(w, mag, 'b-', linewidth=2.5)
ax_mag.set_xlabel('Fréquence (Hz)', fontsize=11)
ax_mag.set_ylabel('Gain (dB)', fontsize=11)
ax_mag.set_title('Réponse en Fréquence - Gain', fontsize=12, fontweight='bold')
ax_mag.grid(True, which='both', alpha=0.3)
ax_mag.set_xlim(20, fs/2)
ax_mag.set_ylim(-50, 20)

# Tracé de la phase
line_phase, = ax_phase.semilogx(w, phase, 'r-', linewidth=2.5)
ax_phase.set_xlabel('Fréquence (Hz)', fontsize=11)
ax_phase.set_ylabel('Phase (degrés)', fontsize=11)
ax_phase.set_title('Réponse en Fréquence - Phase', fontsize=12, fontweight='bold')
ax_phase.grid(True, which='both', alpha=0.3)
ax_phase.set_xlim(20, fs/2)
ax_phase.set_ylim(-360, 360)

# Position des sliders
slider_ax_freq = plt.axes([0.15, 0.16, 0.3, 0.02])
slider_ax_bw = plt.axes([0.15, 0.11, 0.3, 0.02])
slider_ax_gain = plt.axes([0.15, 0.06, 0.3, 0.02])

# Création du slider de fréquence avec échelle logarithmique
# On utilise log10 pour l'échelle interne
log_min = np.log10(20)
log_max = np.log10(20000)
log_initial = np.log10(initial_freq)

slider_freq = Slider(slider_ax_freq, 'Fréquence (Hz)', log_min, log_max, 
                     valinit=log_initial, valfmt='%.0f Hz')
slider_bw = Slider(slider_ax_bw, 'Bandwidth', 0.1, 4.0, 
                   valinit=initial_bandwidth, valstep=0.01, valfmt='%.2f')
slider_gain = Slider(slider_ax_gain, 'Gain (dB)', -24, 24, 
                     valinit=initial_gain, valstep=0.5, valfmt='%.1f dB')

# Radio buttons pour le type de filtre
radio_ax = plt.axes([0.60, 0.05, 0.35, 0.17])
radio = RadioButtons(radio_ax, 
                     ('LPF (12dB/oct)', 'LPF24 (24dB/oct)', 
                      'HPF (12dB/oct)', 'HPF24 (24dB/oct)',
                      'BPF', 'NOTCH', 'PEQ', 'LSH', 'HSH', 'AFP'),
                     active=0)

# Mapping des labels vers les types de filtre
filter_type_map = {
    'LPF (12dB/oct)': 'LPF',
    'LPF24 (24dB/oct)': 'LPF24',
    'HPF (12dB/oct)': 'HPF',
    'HPF24 (24dB/oct)': 'HPF24',
    'BPF': 'BPF',
    'NOTCH': 'NOTCH',
    'PEQ': 'PEQ',
    'LSH': 'LSH',
    'HSH': 'HSH',
    'AFP': 'AFP'
}

# Fonction de formatage pour afficher la fréquence en Hz
def freq_format(x):
    return f'{10**x:.0f} Hz'

slider_freq.valtext.set_text(freq_format(log_initial))

# Fonction de mise à jour
def update(val=None):
    freq = 10 ** slider_freq.val  # Conversion de log vers linéaire
    slider_freq.valtext.set_text(freq_format(slider_freq.val))
    bandwidth = slider_bw.val
    gain = slider_gain.val
    filter_type = filter_type_map[radio.value_selected]
    
    biquad.filter_type = filter_type
    biquad.cutoff_freq = freq
    biquad.bandwidth = bandwidth
    biquad.gain_db = gain
    biquad.calculate_coefficients()
    
    w, mag, phase = biquad.get_frequency_response()
    line_mag.set_data(w, mag)
    line_phase.set_data(w, phase)
    
    # Ajustement automatique de l'échelle Y pour le gain
    y_min = max(np.min(mag) - 10, -80)
    y_max = min(np.max(mag) + 10, 50)
    ax_mag.set_ylim(y_min, y_max)
    
    # Ajustement de l'échelle Y pour la phase (24dB/oct va au-delà de ±180°)
    if filter_type in ['LPF24', 'HPF24']:
        ax_phase.set_ylim(-360, 360)
    else:
        ax_phase.set_ylim(-180, 180)
    
    fig.canvas.draw_idle()

# Connexion des événements
slider_freq.on_changed(update)
slider_bw.on_changed(update)
slider_gain.on_changed(update)
radio.on_clicked(update)

plt.suptitle('Analyseur de Filtres Biquad - DadDSP', fontsize=16, fontweight='bold')
plt.show()