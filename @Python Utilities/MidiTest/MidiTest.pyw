import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import mido
import os
import json

class PotentiometerMIDI:
    def __init__(self, root):
        self.root = root
        self.root.title("Contrôleur MIDI Multi-Onglets")
        self.root.geometry("520x820")
        self.root.configure(bg='#2b2b2b')

        # === Paramètres généraux ===
        self.num_tabs = 5
        self.sliders_per_tab = 6
        self.slider_values = [[0] * self.sliders_per_tab for _ in range(self.num_tabs)]
        self.slider_cc = [[i * self.sliders_per_tab + j + 1 for j in range(self.sliders_per_tab)]
                          for i in range(self.num_tabs)]

        self.midi_output = None
        self.midi_channel = 0  # 0 à 15
        self.omni_mode = False

        # Dossier de configuration
        self.config_dir = os.path.join(os.path.expanduser("~"), ".midi_controller")
        os.makedirs(self.config_dir, exist_ok=True)
        self.default_config_file = os.path.join(self.config_dir, "default_config.json")

        # Stockage des widgets
        self.sliders = [[] for _ in range(self.num_tabs)]
        self.value_labels = [[] for _ in range(self.num_tabs)]
        self.cc_entries = [[] for _ in range(self.num_tabs)]

        # Style
        self.setup_styles()

        # UI (DOIT être appelé AVANT setup_midi)
        self.setup_ui()

        # Charger config par défaut au démarrage
        self.load_default_config()

        # MIDI (DOIT être appelé APRÈS setup_ui)
        self.setup_midi()

    def setup_styles(self):
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('Custom.TNotebook', background='#2b2b2b', tabmargins=[2, 5, 2, 0])
        style.configure('Custom.TNotebook.Tab',
                        background='#3b3b3b', foreground='white', padding=[12, 6])
        style.map('Custom.TNotebook.Tab',
                  background=[('selected', '#4b4b4b'), ('active', '#5b5b5b')],
                  foreground=[('selected', 'white'), ('active', 'white')])

    def setup_ui(self):
        # === Notebook (onglets) ===
        self.notebook = ttk.Notebook(self.root, style='Custom.TNotebook')
        self.notebook.pack(pady=10, padx=10, fill=tk.BOTH, expand=True)

        # === Création des 5 onglets CC ===
        for i in range(self.num_tabs):
            tab = tk.Frame(self.notebook, bg='#2b2b2b')
            self.notebook.add(tab, text=f"CC {i+1}")
            self.create_tab_sliders(tab, i)

        # === Onglet Program Change ===
        self.create_program_change_tab()

        # === Sélection MIDI ===
        midi_frame = tk.Frame(self.root, bg='#2b2b2b')
        midi_frame.pack(pady=8, fill=tk.X, padx=10)

        # Interface
        tk.Label(midi_frame, text="Interface :", bg='#2b2b2b', fg='white', font=("Arial", 9)).pack(side=tk.LEFT)
        self.midi_interfaces = self.get_midi_interfaces()
        self.interface_var = tk.StringVar(value=self.midi_interfaces[0] if self.midi_interfaces else "")
        self.interface_combo = ttk.Combobox(midi_frame, textvariable=self.interface_var,
                                       values=self.midi_interfaces, state='readonly', width=25)
        self.interface_combo.pack(side=tk.LEFT, padx=8)
        self.interface_combo.bind('<<ComboboxSelected>>', self.change_midi_interface)

        # Port
        tk.Label(midi_frame, text="Port :", bg='#2b2b2b', fg='white', font=("Arial", 9)).pack(side=tk.LEFT)
        self.midi_ports = []
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(midi_frame, textvariable=self.port_var,
                                       values=self.midi_ports, state='readonly', width=30)
        self.port_combo.pack(side=tk.LEFT, padx=8)
        self.port_combo.bind('<<ComboboxSelected>>', self.change_midi_port)

        # Bouton Refresh
        tk.Button(midi_frame, text="Refresh", command=self.refresh_midi,
                  bg='#4b4b4b', fg='white', width=8).pack(side=tk.RIGHT)

        # === Canal MIDI + Omni ===
        chan_frame = tk.Frame(self.root, bg='#2b2b2b')
        chan_frame.pack(pady=5)
        tk.Label(chan_frame, text="Canal MIDI :", bg='#2b2b2b', fg='white').pack(side=tk.LEFT, padx=10)
        self.channel_var = tk.StringVar(value="1")
        tk.Spinbox(chan_frame, from_=1, to=16, textvariable=self.channel_var, width=4,
                   command=self.update_midi_channel).pack(side=tk.LEFT, padx=5)

        self.omni_var = tk.BooleanVar()
        tk.Checkbutton(chan_frame, text="Omni (tous canaux)", variable=self.omni_var,
                       command=self.toggle_omni_mode, bg='#2b2b2b', fg='white',
                       selectcolor='#3b3b3b').pack(side=tk.LEFT, padx=15)

        # === Boutons principaux ===
        btn_frame1 = tk.Frame(self.root, bg='#2b2b2b')
        btn_frame1.pack(pady=8)
        tk.Button(btn_frame1, text="Send Tab", command=self.test_current_tab,
                  bg='#4b6b4b', fg='white', width=12).grid(row=0, column=0, padx=4)
        tk.Button(btn_frame1, text="Reset Tab", command=self.reset_current_tab,
                  bg='#6b4b4b', fg='white', width=12).grid(row=0, column=1, padx=4)
        tk.Button(btn_frame1, text="Send All", command=self.test_all_sliders,
                  bg='#4b6b4b', fg='white', width=12).grid(row=0, column=2, padx=4)
        tk.Button(btn_frame1, text="Reset All", command=self.reset_all_sliders,
                  bg='#6b4b4b', fg='white', width=12).grid(row=0, column=3, padx=4)

        # === Sauvegarde / Chargement ===
        save_frame = tk.Frame(self.root, bg='#2b2b2b')
        save_frame.pack(pady=8)
        tk.Button(save_frame, text="Sauvegarder", command=self.save_config_dialog,
                  bg='#2d5c2d', fg='white', width=14).pack(side=tk.LEFT, padx=5)
        tk.Button(save_frame, text="Charger", command=self.load_config_dialog,
                  bg='#2d5c2d', fg='white', width=14).pack(side=tk.LEFT, padx=5)
        tk.Button(save_frame, text="Sauv. Défaut", command=self.save_default_config,
                  bg='#3d6b3d', fg='white', width=12).pack(side=tk.LEFT, padx=5)
        tk.Button(save_frame, text="Char. Défaut", command=self.load_default_config,
                  bg='#3d6b3d', fg='white', width=12).pack(side=tk.LEFT, padx=5)
        tk.Button(save_frame, text="Dossier", command=self.open_config_folder,
                  bg='#3d6b3d', fg='white', width=10).pack(side=tk.LEFT, padx=5)

    def create_program_change_tab(self):
        pc_tab = tk.Frame(self.notebook, bg='#2b2b2b')
        self.notebook.add(pc_tab, text="Program Change")

        tk.Label(pc_tab, text="Program Change", font=("Arial", 18, "bold"),
                 bg='#2b2b2b', fg='white').pack(pady=20)

        main_frame = tk.Frame(pc_tab, bg='#2b2b2b')
        main_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)

        # Sélection du programme
        prog_frame = tk.Frame(main_frame, bg='#2b2b2b')
        prog_frame.pack(pady=20)
        tk.Label(prog_frame, text="Programme :", font=("Arial", 12), bg='#2b2b2b', fg='white').pack(side=tk.LEFT, padx=10)
        self.program_var = tk.StringVar(value="0")
        spin = tk.Spinbox(prog_frame, from_=0, to=127, textvariable=self.program_var,
                          font=("Arial", 14), width=6, justify='center',
                          bg='#3b3b3b', fg='white', buttonbackground='#4b4b4b')
        spin.pack(side=tk.LEFT, padx=10)

        # Bouton envoi
        tk.Button(main_frame, text="Envoyer Program Change",
                  command=self.send_program_change,
                  font=("Arial", 14, "bold"), bg='#2d5c2d', fg='white',
                  width=30, height=2).pack(pady=30)

        # === Presets rapides ===
        preset_labelframe = tk.LabelFrame(main_frame, text=" Presets Rapides ",
                                          font=("Arial", 12, "bold"), bg='#2b2b2b', fg='white')
        preset_labelframe.pack(pady=20, padx=30, fill=tk.X)

        grid = tk.Frame(preset_labelframe, bg='#2b2b2b')
        grid.pack(pady=12)

        for i in range(10):
            prog = i
            btn = tk.Button(grid, text=f"{i+1}\n", width=7, height=2,
                            font=("Arial", 11, "bold"), bg='#3d5d3d', fg='white',
                            command=lambda p=prog: self.set_program(p))
            btn.grid(row=i//5, column=i%5, padx=8, pady=8)

    def set_program(self, program):
        """Met à jour le Spinbox quand on clique sur un preset"""
        self.program_var.set(str(program))

    def send_program_change(self):
        if not self.midi_output:
            messagebox.showerror("Erreur", "Aucune sortie MIDI sélectionnée !")
            return
        try:
            prog = int(self.program_var.get())
            if not 0 <= prog <= 127:
                raise ValueError
            channels = range(16) if self.omni_mode else [self.midi_channel]
            for ch in channels:
                msg = mido.Message('program_change', channel=ch, program=prog)
                self.midi_output.send(msg)
            messagebox.showinfo("Succès", f"Program Change {prog} envoyé sur canal(s) {
                                 'OMNI' if self.omni_mode else self.midi_channel+1}")
        except:
            messagebox.showerror("Erreur", "Programme invalide (0-127)")

    def create_tab_sliders(self, parent, tab_index):
        frame = tk.Frame(parent, bg='#2b2b2b')
        frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)
        for i in range(self.sliders_per_tab):
            self.create_slider(frame, tab_index, i)

    def create_slider(self, parent, tab_index, slider_index):
        row = tk.Frame(parent, bg='#2b2b2b')
        row.pack(fill=tk.X, pady=6)

        tk.Label(row, text=f"S{slider_index+1}:", width=4, bg='#2b2b2b', fg='white').grid(row=0, column=0)

        # CC number
        cc_var = tk.StringVar(value=str(self.slider_cc[tab_index][slider_index]))
        tk.Label(row, text="CC:", bg='#2b2b2b', fg='white').grid(row=0, column=1)
        cc_entry = tk.Entry(row, textvariable=cc_var, width=4, justify='center', bg='#333', fg='white')
        cc_entry.grid(row=0, column=2, padx=5)
        cc_entry.bind('<Return>', lambda e, t=tab_index, s=slider_index: self.update_cc(t, s))
        cc_entry.bind('<FocusOut>', lambda e, t=tab_index, s=slider_index: self.update_cc(t, s))
        self.cc_entries[tab_index].append(cc_var)

        # Value display
        val_var = tk.StringVar(value="0")
        val_entry = tk.Entry(row, textvariable=val_var, width=5, justify='center', bg='#333', fg='#0f0', font=("Arial", 10, "bold"))
        val_entry.grid(row=0, column=3, padx=10)
        val_entry.bind('<Return>', lambda e, t=tab_index, s=slider_index: self.update_value(t, s))
        val_entry.bind('<FocusOut>', lambda e, t=tab_index, s=slider_index: self.update_value(t, s))
        self.value_labels[tab_index].append(val_var)

        # Slider
        slider = tk.Scale(row, from_=0, to=127, orient=tk.HORIZONTAL, length=300,
                          command=lambda v, t=tab_index, s=slider_index: self.on_slider(t, s, v),
                          bg='#333', fg='white', troughcolor='#111', highlightthickness=0)
        slider.grid(row=1, column=0, columnspan=4, pady=5, sticky='ew')
        slider.set(0)
        self.sliders[tab_index].append(slider)

    def on_slider(self, tab, idx, value):
        value = int(float(value))
        self.slider_values[tab][idx] = value
        self.value_labels[tab][idx].set(str(value))
        self.send_midi(tab, idx)

    def update_cc(self, tab, idx):
        try:
            cc = int(self.cc_entries[tab][idx].get())
            if 0 <= cc <= 127:
                self.slider_cc[tab][idx] = cc
            else:
                raise ValueError
        except:
            self.cc_entries[tab][idx].set(str(self.slider_cc[tab][idx]))

    def update_value(self, tab, idx):
        try:
            val = int(self.value_labels[tab][idx].get())
            if 0 <= val <= 127:
                self.sliders[tab][idx].set(val)
                self.slider_values[tab][idx] = val
                self.send_midi(tab, idx)
            else:
                raise ValueError
        except:
            self.value_labels[tab][idx].set(str(self.slider_values[tab][idx]))

    def send_midi(self, tab, idx):
        if not self.midi_output:
            return
        cc = self.slider_cc[tab][idx]
        val = self.slider_values[tab][idx]
        if self.omni_mode:
            for ch in range(16):
                self.midi_output.send(mido.Message('control_change', channel=ch, control=cc, value=val))
        else:
            self.midi_output.send(mido.Message('control_change', channel=self.midi_channel, control=cc, value=val))

    # === Autres fonctions (test, reset, save/load, MIDI, etc.) ===
    def test_current_tab(self):
        idx = self.notebook.index(self.notebook.select())
        if idx < self.num_tabs and self.midi_output:
            for i in range(self.sliders_per_tab):
                self.send_midi(idx, i)

    def reset_current_tab(self):
        idx = self.notebook.index(self.notebook.select())
        if idx < self.num_tabs:
            for i in range(self.sliders_per_tab):
                self.sliders[idx][i].set(0)
                self.slider_values[idx][i] = 0
                self.value_labels[idx][i].set("0")
                self.send_midi(idx, i)

    def test_all_sliders(self):
        if self.midi_output:
            for t in range(self.num_tabs):
                for i in range(self.sliders_per_tab):
                    self.send_midi(t, i)

    def reset_all_sliders(self):
        for t in range(self.num_tabs):
            for i in range(self.sliders_per_tab):
                self.sliders[t][i].set(0)
                self.slider_values[t][i] = 0
                self.value_labels[t][i].set("0")
                self.send_midi(t, i)

    def toggle_omni_mode(self):
        self.omni_mode = self.omni_var.get()

    def update_midi_channel(self):
        try:
            self.midi_channel = int(self.channel_var.get()) - 1
        except:
            self.midi_channel = 0

    # === Gestion MIDI ===
    def get_midi_interfaces(self):
        try:
            ports = mido.get_output_names()
            interfaces = sorted({p.split(':')[0] if ':' in p else p for p in ports})
            return interfaces if interfaces else ["Aucune interface"]
        except:
            return ["Erreur MIDI"]

    def update_midi_ports(self):
        try:
            all_ports = mido.get_output_names()
            iface = self.interface_var.get()
            ports = [p for p in all_ports if iface in p]
            self.midi_ports = ports if ports else ["Aucun port"]
            self.port_combo['values'] = self.midi_ports
            if self.midi_ports:
                self.port_var.set(self.midi_ports[0])
            self.change_midi_port()
        except:
            pass

    def change_midi_interface(self, _=None):
        self.update_midi_ports()

    def change_midi_port(self, _=None):
        if self.midi_output:
            self.midi_output.close()
        port = self.port_var.get()
        if port and port != "Aucun port":
            try:
                self.midi_output = mido.open_output(port)
                print(f"MIDI → {port}")
            except:
                self.midi_output = None

    def refresh_midi(self):
        self.midi_interfaces = self.get_midi_interfaces()
        self.interface_combo['values'] = self.midi_interfaces
        if self.midi_interfaces:
            self.interface_var.set(self.midi_interfaces[0])
        self.update_midi_ports()

    def setup_midi(self):
        self.update_midi_ports()

    # === Sauvegarde / Chargement ===
    def get_config_data(self):
        return {
            'slider_values': self.slider_values,
            'slider_cc': self.slider_cc,
            'midi_channel': self.midi_channel,
            'omni_mode': self.omni_mode,
        }

    def apply_config(self, data):
        try:
            self.slider_values = data.get('slider_values', self.slider_values)
            self.slider_cc = data.get('slider_cc', self.slider_cc)
            for t in range(self.num_tabs):
                for i in range(self.sliders_per_tab):
                    self.sliders[t][i].set(self.slider_values[t][i])
                    self.value_labels[t][i].set(str(self.slider_values[t][i]))
                    self.cc_entries[t][i].set(str(self.slider_cc[t][i]))
            self.midi_channel = data.get('midi_channel', 0)
            self.channel_var.set(str(self.midi_channel + 1))
            self.omni_mode = data.get('omni_mode', False)
            self.omni_var.set(self.omni_mode)
        except:
            pass

    def save_config_dialog(self):
        file = filedialog.asksaveasfilename(initialdir=self.config_dir, defaultextension=".json")
        if file:
            with open(file, 'w') as f:
                json.dump(self.get_config_data(), f, indent=4)
            messagebox.showinfo("OK", "Sauvegardé")

    def load_config_dialog(self):
        file = filedialog.askopenfilename(initialdir=self.config_dir, filetypes=[("JSON", "*.json")])
        if file:
            with open(file) as f:
                self.apply_config(json.load(f))
            messagebox.showinfo("OK", "Chargé")

    def save_default_config(self):
        with open(self.default_config_file, 'w') as f:
            json.dump(self.get_config_data(), f, indent=4)
        messagebox.showinfo("OK", "Configuration par défaut sauvegardée")

    def load_default_config(self):
        if os.path.exists(self.default_config_file):
            with open(self.default_config_file) as f:
                self.apply_config(json.load(f))

    def open_config_folder(self):
        if os.name == 'nt':
            os.startfile(self.config_dir)
        else:
            os.system(f'xdg-open "{self.config_dir}"')

    def cleanup(self):
        if self.midi_output:
            self.midi_output.close()

# === Lancement ===
def main():
    root = tk.Tk()
    app = PotentiometerMIDI(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.cleanup(), root.destroy()))
    root.mainloop()

if __name__ == "__main__":
    main()