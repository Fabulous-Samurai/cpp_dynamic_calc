#!/usr/bin/env python3
"""
🚀 OGULATOR Signal Processing Toolkit 
Advanced signal analysis and processing capabilities
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, ifft, fftfreq
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

class SignalProcessingToolkit:
    """🎛️ Professional Signal Processing Toolkit 🎛️"""
    
    def __init__(self, parent_gui=None):
        self.parent_gui = parent_gui
        self.signals = {}  # Store signals
        self.sampling_rate = 1000  # Default sampling rate
        
    def create_test_signals(self):
        """Create various test signals for demonstration"""
        t = np.linspace(0, 1, self.sampling_rate, endpoint=False)
        
        # Test signals
        self.signals = {
            'sine_wave': np.sin(2 * np.pi * 50 * t),  # 50 Hz sine
            'cosine_wave': np.cos(2 * np.pi * 30 * t),  # 30 Hz cosine
            'square_wave': signal.square(2 * np.pi * 25 * t),  # 25 Hz square
            'sawtooth_wave': signal.sawtooth(2 * np.pi * 40 * t),  # 40 Hz sawtooth
            'chirp_signal': signal.chirp(t, f0=10, f1=100, t1=1, method='linear'),
            'noise': np.random.normal(0, 0.1, len(t)),
            'mixed_signal': (np.sin(2 * np.pi * 50 * t) + 
                           0.5 * np.sin(2 * np.pi * 120 * t) + 
                           0.2 * np.random.normal(0, 1, len(t)))
        }
        
        self.time_vector = t
        return self.signals
    
    def generate_signal_gui(self):
        """Open signal generation GUI"""
        gen_window = tk.Toplevel()
        gen_window.title("🎛️ Signal Generator")
        gen_window.geometry("600x500")
        
        # Signal type selection
        ttk.Label(gen_window, text="Signal Type:", font=('Segoe UI', 12, 'bold')).pack(pady=10)
        
        signal_frame = ttk.Frame(gen_window)
        signal_frame.pack(fill=tk.X, padx=20, pady=10)
        
        signal_types = [
            "Sine Wave", "Cosine Wave", "Square Wave", 
            "Sawtooth Wave", "Triangle Wave", "Chirp Signal",
            "White Noise", "Pink Noise", "Impulse"
        ]
        
        signal_var = tk.StringVar(value="Sine Wave")
        for i, sig_type in enumerate(signal_types):
            ttk.Radiobutton(signal_frame, text=sig_type, 
                           variable=signal_var, value=sig_type).grid(
                           row=i//3, column=i%3, sticky='w', padx=10, pady=2)
        
        # Parameters frame
        params_frame = ttk.LabelFrame(gen_window, text="Parameters")
        params_frame.pack(fill=tk.X, padx=20, pady=10)
        
        # Frequency
        ttk.Label(params_frame, text="Frequency (Hz):").grid(row=0, column=0, sticky='w', padx=5, pady=5)
        freq_var = tk.DoubleVar(value=50.0)
        ttk.Entry(params_frame, textvariable=freq_var, width=15).grid(row=0, column=1, padx=5, pady=5)
        
        # Amplitude
        ttk.Label(params_frame, text="Amplitude:").grid(row=0, column=2, sticky='w', padx=5, pady=5)
        amp_var = tk.DoubleVar(value=1.0)
        ttk.Entry(params_frame, textvariable=amp_var, width=15).grid(row=0, column=3, padx=5, pady=5)
        
        # Phase
        ttk.Label(params_frame, text="Phase (rad):").grid(row=1, column=0, sticky='w', padx=5, pady=5)
        phase_var = tk.DoubleVar(value=0.0)
        ttk.Entry(params_frame, textvariable=phase_var, width=15).grid(row=1, column=1, padx=5, pady=5)
        
        # Duration
        ttk.Label(params_frame, text="Duration (s):").grid(row=1, column=2, sticky='w', padx=5, pady=5)
        duration_var = tk.DoubleVar(value=1.0)
        ttk.Entry(params_frame, textvariable=duration_var, width=15).grid(row=1, column=3, padx=5, pady=5)
        
        # Sampling rate
        ttk.Label(params_frame, text="Sampling Rate (Hz):").grid(row=2, column=0, sticky='w', padx=5, pady=5)
        fs_var = tk.DoubleVar(value=1000.0)
        ttk.Entry(params_frame, textvariable=fs_var, width=15).grid(row=2, column=1, padx=5, pady=5)
        
        # Signal name
        ttk.Label(params_frame, text="Signal Name:").grid(row=2, column=2, sticky='w', padx=5, pady=5)
        name_var = tk.StringVar(value="generated_signal")
        ttk.Entry(params_frame, textvariable=name_var, width=15).grid(row=2, column=3, padx=5, pady=5)
        
        def generate_signal():
            """Generate signal based on parameters"""
            try:
                freq = freq_var.get()
                amp = amp_var.get()
                phase = phase_var.get()
                duration = duration_var.get()
                fs = fs_var.get()
                name = name_var.get()
                sig_type = signal_var.get()
                
                # Time vector
                t = np.linspace(0, duration, int(fs * duration), endpoint=False)
                
                # Generate signal
                if sig_type == "Sine Wave":
                    sig = amp * np.sin(2 * np.pi * freq * t + phase)
                elif sig_type == "Cosine Wave":
                    sig = amp * np.cos(2 * np.pi * freq * t + phase)
                elif sig_type == "Square Wave":
                    sig = amp * signal.square(2 * np.pi * freq * t + phase)
                elif sig_type == "Sawtooth Wave":
                    sig = amp * signal.sawtooth(2 * np.pi * freq * t + phase)
                elif sig_type == "Triangle Wave":
                    sig = amp * signal.sawtooth(2 * np.pi * freq * t + phase, 0.5)
                elif sig_type == "Chirp Signal":
                    sig = amp * signal.chirp(t, f0=freq, f1=freq*2, t1=duration, method='linear')
                elif sig_type == "White Noise":
                    sig = amp * np.random.normal(0, 1, len(t))
                elif sig_type == "Pink Noise":
                    white = np.random.normal(0, 1, len(t))
                    sig = amp * self.pink_noise_filter(white)
                elif sig_type == "Impulse":
                    sig = np.zeros(len(t))
                    sig[0] = amp
                
                # Store signal
                self.signals[name] = sig
                self.time_vector = t
                self.sampling_rate = fs
                
                messagebox.showinfo("Success", f"Signal '{name}' generated successfully!")
                gen_window.destroy()
                
            except Exception as e:
                messagebox.showerror("Error", f"Signal generation failed: {str(e)}")
        
        # Generate button
        ttk.Button(gen_window, text="🎯 Generate Signal", 
                  command=generate_signal).pack(pady=20)
    
    def pink_noise_filter(self, white_noise):
        """Generate pink noise from white noise"""
        # Simple pink noise approximation using filtering
        b, a = signal.butter(1, 0.1, btype='low')
        return signal.filtfilt(b, a, white_noise)
    
    def frequency_analysis(self, signal_name):
        """Perform frequency domain analysis"""
        if signal_name not in self.signals:
            print(f"Signal '{signal_name}' not found!")
            return
        
        sig = self.signals[signal_name]
        
        # FFT analysis
        fft_result = fft(sig)
        frequencies = fftfreq(len(sig), 1/self.sampling_rate)
        magnitude = np.abs(fft_result)
        phase = np.angle(fft_result)
        
        # Power spectral density
        f_psd, psd = signal.welch(sig, self.sampling_rate, nperseg=256)
        
        # Create comprehensive frequency analysis plot
        fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
        fig.suptitle(f'🎛️ Frequency Analysis: {signal_name}', fontsize=16, fontweight='bold')
        
        # Time domain
        ax1.plot(self.time_vector, sig, 'b-', linewidth=1)
        ax1.set_title('Time Domain')
        ax1.set_xlabel('Time (s)')
        ax1.set_ylabel('Amplitude')
        ax1.grid(True, alpha=0.3)
        
        # Magnitude spectrum
        ax2.plot(frequencies[:len(frequencies)//2], magnitude[:len(magnitude)//2], 'r-', linewidth=1)
        ax2.set_title('Magnitude Spectrum')
        ax2.set_xlabel('Frequency (Hz)')
        ax2.set_ylabel('Magnitude')
        ax2.grid(True, alpha=0.3)
        
        # Phase spectrum
        ax3.plot(frequencies[:len(frequencies)//2], phase[:len(phase)//2], 'g-', linewidth=1)
        ax3.set_title('Phase Spectrum')
        ax3.set_xlabel('Frequency (Hz)')
        ax3.set_ylabel('Phase (rad)')
        ax3.grid(True, alpha=0.3)
        
        # Power spectral density
        ax4.semilogy(f_psd, psd, 'm-', linewidth=2)
        ax4.set_title('Power Spectral Density')
        ax4.set_xlabel('Frequency (Hz)')
        ax4.set_ylabel('PSD (V²/Hz)')
        ax4.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.show()
        
        return {
            'frequencies': frequencies,
            'magnitude': magnitude,
            'phase': phase,
            'psd_frequencies': f_psd,
            'psd': psd
        }
    
    def filter_design_gui(self):
        """Open filter design GUI"""
        filter_window = tk.Toplevel()
        filter_window.title("🎛️ Filter Design")
        filter_window.geometry("700x600")
        
        # Filter type
        ttk.Label(filter_window, text="Filter Design", font=('Segoe UI', 14, 'bold')).pack(pady=10)
        
        type_frame = ttk.LabelFrame(filter_window, text="Filter Type")
        type_frame.pack(fill=tk.X, padx=20, pady=10)
        
        filter_type_var = tk.StringVar(value="lowpass")
        filter_types = ["lowpass", "highpass", "bandpass", "bandstop"]
        
        for i, ftype in enumerate(filter_types):
            ttk.Radiobutton(type_frame, text=ftype.title(), 
                           variable=filter_type_var, value=ftype).grid(
                           row=0, column=i, padx=10, pady=5)
        
        # Filter parameters
        params_frame = ttk.LabelFrame(filter_window, text="Parameters")
        params_frame.pack(fill=tk.X, padx=20, pady=10)
        
        ttk.Label(params_frame, text="Filter Order:").grid(row=0, column=0, sticky='w', padx=5, pady=5)
        order_var = tk.IntVar(value=4)
        ttk.Entry(params_frame, textvariable=order_var, width=15).grid(row=0, column=1, padx=5, pady=5)
        
        ttk.Label(params_frame, text="Cutoff Freq (Hz):").grid(row=0, column=2, sticky='w', padx=5, pady=5)
        cutoff_var = tk.DoubleVar(value=100.0)
        ttk.Entry(params_frame, textvariable=cutoff_var, width=15).grid(row=0, column=3, padx=5, pady=5)
        
        ttk.Label(params_frame, text="Low Freq (Hz):").grid(row=1, column=0, sticky='w', padx=5, pady=5)
        low_freq_var = tk.DoubleVar(value=50.0)
        ttk.Entry(params_frame, textvariable=low_freq_var, width=15).grid(row=1, column=1, padx=5, pady=5)
        
        ttk.Label(params_frame, text="High Freq (Hz):").grid(row=1, column=2, sticky='w', padx=5, pady=5)
        high_freq_var = tk.DoubleVar(value=200.0)
        ttk.Entry(params_frame, textvariable=high_freq_var, width=15).grid(row=1, column=3, padx=5, pady=5)
        
        # Filter design method
        method_frame = ttk.LabelFrame(filter_window, text="Design Method")
        method_frame.pack(fill=tk.X, padx=20, pady=10)
        
        method_var = tk.StringVar(value="butterworth")
        methods = ["butterworth", "chebyshev1", "chebyshev2", "elliptic"]
        
        for i, method in enumerate(methods):
            ttk.Radiobutton(method_frame, text=method.title(), 
                           variable=method_var, value=method).grid(
                           row=0, column=i, padx=10, pady=5)
        
        def design_filter():
            """Design and plot filter"""
            try:
                order = order_var.get()
                cutoff = cutoff_var.get()
                low_freq = low_freq_var.get()
                high_freq = high_freq_var.get()
                ftype = filter_type_var.get()
                method = method_var.get()
                
                # Normalize frequencies
                nyquist = self.sampling_rate / 2
                
                if ftype in ['lowpass', 'highpass']:
                    critical = cutoff / nyquist
                else:  # bandpass, bandstop
                    critical = [low_freq / nyquist, high_freq / nyquist]
                
                # Design filter
                if method == "butterworth":
                    b, a = signal.butter(order, critical, btype=ftype)
                elif method == "chebyshev1":
                    b, a = signal.cheby1(order, 1, critical, btype=ftype)
                elif method == "chebyshev2":
                    b, a = signal.cheby2(order, 20, critical, btype=ftype)
                elif method == "elliptic":
                    b, a = signal.ellip(order, 1, 20, critical, btype=ftype)
                
                # Plot filter response
                w, h = signal.freqz(b, a, worN=8000)
                frequencies = w * self.sampling_rate / (2 * np.pi)
                
                fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
                fig.suptitle(f'🎛️ {method.title()} {ftype.title()} Filter Response', 
                           fontsize=14, fontweight='bold')
                
                # Magnitude response
                ax1.plot(frequencies, 20 * np.log10(np.abs(h)), 'b-', linewidth=2)
                ax1.set_title('Magnitude Response')
                ax1.set_xlabel('Frequency (Hz)')
                ax1.set_ylabel('Magnitude (dB)')
                ax1.grid(True, alpha=0.3)
                ax1.set_xlim(0, self.sampling_rate/2)
                
                # Phase response
                ax2.plot(frequencies, np.angle(h), 'r-', linewidth=2)
                ax2.set_title('Phase Response')
                ax2.set_xlabel('Frequency (Hz)')
                ax2.set_ylabel('Phase (rad)')
                ax2.grid(True, alpha=0.3)
                ax2.set_xlim(0, self.sampling_rate/2)
                
                # Group delay
                w_gd, gd = signal.group_delay((b, a))
                freq_gd = w_gd * self.sampling_rate / (2 * np.pi)
                ax3.plot(freq_gd, gd, 'g-', linewidth=2)
                ax3.set_title('Group Delay')
                ax3.set_xlabel('Frequency (Hz)')
                ax3.set_ylabel('Group Delay (samples)')
                ax3.grid(True, alpha=0.3)
                ax3.set_xlim(0, self.sampling_rate/2)
                
                plt.tight_layout()
                plt.show()
                
                # Store filter coefficients
                self.filter_coeffs = (b, a)
                messagebox.showinfo("Success", "Filter designed successfully!")
                
            except Exception as e:
                messagebox.showerror("Error", f"Filter design failed: {str(e)}")
        
        # Design button
        ttk.Button(filter_window, text="🎯 Design Filter", 
                  command=design_filter).pack(pady=20)
    
    def apply_filter(self, signal_name, filter_coeffs=None):
        """Apply filter to signal"""
        if signal_name not in self.signals:
            print(f"Signal '{signal_name}' not found!")
            return
        
        if filter_coeffs is None and not hasattr(self, 'filter_coeffs'):
            print("No filter designed! Use filter_design_gui() first.")
            return
        
        if filter_coeffs is None:
            filter_coeffs = self.filter_coeffs
        
        b, a = filter_coeffs
        sig = self.signals[signal_name]
        
        # Apply filter
        filtered_sig = signal.filtfilt(b, a, sig)
        
        # Plot comparison
        fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
        fig.suptitle(f'🎛️ Filter Application: {signal_name}', fontsize=14, fontweight='bold')
        
        # Original signal
        ax1.plot(self.time_vector, sig, 'b-', linewidth=1, label='Original')
        ax1.set_title('Original Signal')
        ax1.set_xlabel('Time (s)')
        ax1.set_ylabel('Amplitude')
        ax1.grid(True, alpha=0.3)
        ax1.legend()
        
        # Filtered signal
        ax2.plot(self.time_vector, filtered_sig, 'r-', linewidth=1, label='Filtered')
        ax2.set_title('Filtered Signal')
        ax2.set_xlabel('Time (s)')
        ax2.set_ylabel('Amplitude')
        ax2.grid(True, alpha=0.3)
        ax2.legend()
        
        # Comparison
        ax3.plot(self.time_vector, sig, 'b-', linewidth=1, label='Original', alpha=0.7)
        ax3.plot(self.time_vector, filtered_sig, 'r-', linewidth=1, label='Filtered')
        ax3.set_title('Comparison')
        ax3.set_xlabel('Time (s)')
        ax3.set_ylabel('Amplitude')
        ax3.grid(True, alpha=0.3)
        ax3.legend()
        
        plt.tight_layout()
        plt.show()
        
        # Store filtered signal
        self.signals[f"{signal_name}_filtered"] = filtered_sig
        
        return filtered_sig
    
    def spectral_analysis_gui(self):
        """Advanced spectral analysis GUI"""
        spec_window = tk.Toplevel()
        spec_window.title("🎛️ Spectral Analysis")
        spec_window.geometry("800x600")
        
        ttk.Label(spec_window, text="Spectral Analysis Tools", 
                 font=('Segoe UI', 14, 'bold')).pack(pady=10)
        
        # Signal selection
        signal_frame = ttk.LabelFrame(spec_window, text="Select Signal")
        signal_frame.pack(fill=tk.X, padx=20, pady=10)
        
        signal_var = tk.StringVar()
        signal_combo = ttk.Combobox(signal_frame, textvariable=signal_var, 
                                   values=list(self.signals.keys()))
        signal_combo.pack(pady=10)
        
        # Analysis options
        analysis_frame = ttk.LabelFrame(spec_window, text="Analysis Options")
        analysis_frame.pack(fill=tk.X, padx=20, pady=10)
        
        analysis_buttons = [
            ("📊 FFT Analysis", lambda: self.fft_analysis(signal_var.get())),
            ("🎵 Spectrogram", lambda: self.spectrogram_analysis(signal_var.get())),
            ("📈 Power Spectral Density", lambda: self.psd_analysis(signal_var.get())),
            ("🎯 Peak Detection", lambda: self.peak_detection(signal_var.get())),
            ("📐 Cross-Correlation", lambda: self.correlation_analysis(signal_var.get())),
            ("🌊 Wavelet Transform", lambda: self.wavelet_analysis(signal_var.get()))
        ]
        
        for i, (text, command) in enumerate(analysis_buttons):
            ttk.Button(analysis_frame, text=text, command=command).grid(
                row=i//2, column=i%2, padx=10, pady=5, sticky='ew')
        
        analysis_frame.grid_columnconfigure(0, weight=1)
        analysis_frame.grid_columnconfigure(1, weight=1)
    
    def spectrogram_analysis(self, signal_name):
        """Generate spectrogram"""
        if signal_name not in self.signals:
            messagebox.showerror("Error", f"Signal '{signal_name}' not found!")
            return
        
        sig = self.signals[signal_name]
        
        # Compute spectrogram
        f, t, Sxx = signal.spectrogram(sig, self.sampling_rate, nperseg=256)
        
        # Plot spectrogram
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
        fig.suptitle(f'🎛️ Spectrogram Analysis: {signal_name}', fontsize=14, fontweight='bold')
        
        # Time domain signal
        ax1.plot(self.time_vector, sig, 'b-', linewidth=1)
        ax1.set_title('Time Domain Signal')
        ax1.set_xlabel('Time (s)')
        ax1.set_ylabel('Amplitude')
        ax1.grid(True, alpha=0.3)
        
        # Spectrogram
        im = ax2.pcolormesh(t, f, 10 * np.log10(Sxx), shading='gouraud', cmap='jet')
        ax2.set_title('Spectrogram')
        ax2.set_xlabel('Time (s)')
        ax2.set_ylabel('Frequency (Hz)')
        plt.colorbar(im, ax=ax2, label='Power (dB)')
        
        plt.tight_layout()
        plt.show()
    
    def psd_analysis(self, signal_name):
        """Power spectral density analysis"""
        if signal_name not in self.signals:
            messagebox.showerror("Error", f"Signal '{signal_name}' not found!")
            return
        
        sig = self.signals[signal_name]
        
        # Different PSD methods
        f_welch, psd_welch = signal.welch(sig, self.sampling_rate, nperseg=256)
        f_periodogram, psd_periodogram = signal.periodogram(sig, self.sampling_rate)
        
        # Plot comparison
        fig, ax = plt.subplots(1, 1, figsize=(12, 6))
        fig.suptitle(f'🎛️ Power Spectral Density: {signal_name}', fontsize=14, fontweight='bold')
        
        ax.semilogy(f_welch, psd_welch, 'b-', linewidth=2, label='Welch Method')
        ax.semilogy(f_periodogram, psd_periodogram, 'r-', linewidth=1, alpha=0.7, label='Periodogram')
        ax.set_xlabel('Frequency (Hz)')
        ax.set_ylabel('PSD (V²/Hz)')
        ax.grid(True, alpha=0.3)
        ax.legend()
        
        plt.tight_layout()
        plt.show()
    
    def demonstrate_toolkit(self):
        """Demonstrate signal processing capabilities"""
        print("🎛️ OGULATOR Signal Processing Toolkit Demonstration")
        print("=" * 60)
        
        # Create test signals
        self.create_test_signals()
        print(f"✅ Generated {len(self.signals)} test signals")
        
        # Analyze mixed signal
        print("\n📊 Analyzing mixed signal...")
        self.frequency_analysis('mixed_signal')
        
        print("\n🎯 Signal Processing Toolkit Ready!")
        print("Available signals:", list(self.signals.keys()))

if __name__ == "__main__":
    print("🚀 Starting Signal Processing Toolkit...")
    
    # Create toolkit
    toolkit = SignalProcessingToolkit()
    
    # Demonstrate capabilities
    toolkit.demonstrate_toolkit()
    
    # Start GUI for interactive use
    root = tk.Tk()
    root.withdraw()  # Hide main window
    
    # Open signal generator
    toolkit.generate_signal_gui()
    
    root.mainloop()