import tkinter as tk
from tkinter import ttk
import mido
import threading
import time
import math
import json
import os
from tkinter import filedialog, messagebox

class PotentiometerMIDI:
    def __init__(self, root):
        self.root = root
        self.root.title("Contrôleur MIDI Multi-Onglets")
        self.root.geometry("450x750")  # Légèrement augmenté pour voir tous les boutons
        self.root.configure(bg='#2b2b2b')
        
        # Variables
        self.num_tabs = 5
        self.sliders_per_tab = 6
        self.slider_values = [[0] * self.sliders_per_tab for _ in range(self.num_tabs)]
        self.slider_cc = [[i*self.sliders_per_tab + j + 1 for j in range(self.sliders_per_tab)] 
                          for i in range(self.num_tabs)]
        
        self.midi_output = None
        self.midi_channel = 0
        self.omni_mode = False
        
        # Dossier de configuration
        self.config_dir = os.path.join(os.path.expanduser("~"), ".midi_controller")
        if not os.path.exists(self.config_dir):
            os.makedirs(self.config_dir)
        
        self.default_config_file = os.path.join(self.config_dir, "default_config.json")
        
        # Stockage des widgets
        self.sliders = [[] for _ in range(self.num_tabs)]
        self.value_labels = [[] for _ in range(self.num_tabs)]
        self.cc_entries = [[] for _ in range(self.num_tabs)]
        
        # Style pour les onglets
        self.setup_styles()
        
        # Interface graphique
        self.setup_ui()
        
        # Charger la configuration par défaut si elle existe
        self.load_default_config()
        
        # Initialiser MIDI
        self.setup_midi()
    
    def setup_styles(self):
        """Configurer le style des onglets"""
        style = ttk.Style()
        style.theme_use('clam')
        
        # Configuration des couleurs des onglets
        style.configure('Custom.TNotebook', 
                       background='#2b2b2b',
                       tabmargins=[2, 5, 2, 0])
        
        style.configure('Custom.TNotebook.Tab',
                       background='#3b3b3b',
                       foreground='white',
                       padding=[10, 5],
                       focuscolor='none')
        
        style.map('Custom.TNotebook.Tab',
                 background=[('selected', '#4b4b4b'),
                           ('active', '#5b5b5b')],
                 foreground=[('selected', 'white'),
                           ('active', 'white')])
    
    def setup_ui(self):
        # Frame principal pour les onglets
        self.notebook = ttk.Notebook(self.root, style='Custom.TNotebook')
        self.notebook.pack(pady=10, fill=tk.BOTH, expand=True, padx=10)
        
        # Créer les 5 onglets
        self.tab_frames = []
        for i in range(self.num_tabs):
            tab = tk.Frame(self.notebook, bg='#2b2b2b')
            self.notebook.add(tab, text=f"CC{i + 1}")
            self.tab_frames.append(tab)
            
            # Créer les sliders pour cet onglet
            self.create_tab_sliders(tab, i)
        
        # Onglet Program Change
        self.create_program_change_tab()
        
        # Frame pour les sélections MIDI (Interface et Port alignés)
        midi_selection_frame = tk.Frame(self.root, bg='#2b2b2b')
        midi_selection_frame.pack(pady=5, fill=tk.X, padx=10)
        
        # Interface MIDI à gauche
        interface_frame = tk.Frame(midi_selection_frame, bg='#2b2b2b')
        interface_frame.pack(side=tk.LEFT, fill=tk.X, expand=True)
        
        tk.Label(interface_frame, text="Interface:", 
                font=("Arial", 9), bg='#2b2b2b', fg='white').pack(anchor='w')
        
        self.midi_interfaces = self.get_midi_interfaces()
        self.interface_var = tk.StringVar()
        
        if self.midi_interfaces:
            self.interface_var.set(self.midi_interfaces[0])
        else:
            self.midi_interfaces = ["Aucune interface disponible"]
            self.interface_var.set(self.midi_interfaces[0])
        
        self.interface_combo = ttk.Combobox(interface_frame, textvariable=self.interface_var,
                                           values=self.midi_interfaces, state='readonly', width=20)
        self.interface_combo.pack(fill=tk.X, pady=(2, 0))
        self.interface_combo.bind('<<ComboboxSelected>>', self.change_midi_interface)
        
        # Bouton Refresh à droite des listbox
        refresh_btn = tk.Button(midi_selection_frame, text="🔄 Select",
                               command=self.refresh_midi,
                               bg='#4b4b4b', fg='white', 
                               font=("Arial", 9), width=8)
        refresh_btn.pack(side=tk.RIGHT, padx=(5, 0))
        
        # Port MIDI à droite
        port_frame = tk.Frame(midi_selection_frame, bg='#2b2b2b')
        port_frame.pack(side=tk.RIGHT, fill=tk.X, expand=True, padx=(5, 0))
        
        tk.Label(port_frame, text="Port:", 
                font=("Arial", 9), bg='#2b2b2b', fg='white').pack(anchor='w')
        
        self.midi_ports = []
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(port_frame, textvariable=self.port_var,
                                      values=self.midi_ports, state='readonly', width=20)
        self.port_combo.pack(fill=tk.X, pady=(2, 0))
        self.port_combo.bind('<<ComboboxSelected>>', self.change_midi_port)
        
        # Configuration MIDI avancée
        config_frame = tk.Frame(self.root, bg='#2b2b2b')
        config_frame.pack(pady=5, fill=tk.X, padx=10)
        
        # Canal MIDI
        tk.Label(config_frame, text="Canal MIDI:", 
                font=("Arial", 9), bg='#2b2b2b', fg='white').grid(row=0, column=0, padx=5, sticky='w')
        
        self.channel_var = tk.StringVar(value="1")
        channel_spin = tk.Spinbox(config_frame, from_=1, to=16, textvariable=self.channel_var,
                                 width=5, command=self.update_midi_channel,
                                 bg='#3b3b3b', fg='white', justify='center')
        channel_spin.grid(row=0, column=1, padx=5)
        
        # Canal de diffusion (Omni Channel)
        self.omni_var = tk.BooleanVar(value=False)
        omni_check = tk.Checkbutton(config_frame, text="Omni",
                                   variable=self.omni_var, command=self.toggle_omni_mode,
                                   bg='#2b2b2b', fg='white', selectcolor='#3b3b3b',
                                   activebackground='#2b2b2b', activeforeground='white',
                                   font=("Arial", 9))
        omni_check.grid(row=0, column=2, padx=5)
        
        # Boutons principaux - Première ligne
        button_frame1 = tk.Frame(self.root, bg='#2b2b2b')
        button_frame1.pack(pady=5, fill=tk.X, padx=10)
        
        test_btn = tk.Button(button_frame1, text="🎵 Send Tab",
                            command=self.test_current_tab,
                            bg='#4b4b4b', fg='white',
                            font=("Arial", 9), width=12)
        test_btn.pack(side=tk.LEFT, padx=2)
        
        reset_btn = tk.Button(button_frame1, text="🔄 Reset Tab",
                             command=self.reset_current_tab,
                             bg='#4b4b4b', fg='white',
                             font=("Arial", 9), width=12)
        reset_btn.pack(side=tk.LEFT, padx=2)
        
        test_all_btn = tk.Button(button_frame1, text="🎵 Send All",
                                command=self.test_all_sliders,
                                bg='#4b4b4b', fg='white',
                                font=("Arial", 9), width=12)
        test_all_btn.pack(side=tk.LEFT, padx=2)
        
        reset_all_btn = tk.Button(button_frame1, text="🔄 Reset All",
                                 command=self.reset_all_sliders,
                                 bg='#4b4b4b', fg='white',
                                 font=("Arial", 9), width=12)
        reset_all_btn.pack(side=tk.LEFT, padx=2)
        
        # Frame pour les boutons de sauvegarde/chargement
        save_load_frame = tk.Frame(self.root, bg='#2b2b2b')
        save_load_frame.pack(pady=5, fill=tk.X, padx=10)
        
        save_btn = tk.Button(save_load_frame, text="💾 Sauvegarder",
                            command=self.save_config_dialog,
                            bg='#2d5c2d', fg='white',
                            font=("Arial", 9), width=14)
        save_btn.pack(side=tk.LEFT, padx=2)
        
        load_btn = tk.Button(save_load_frame, text="📂 Charger",
                            command=self.load_config_dialog,
                            bg='#2d5c2d', fg='white',
                            font=("Arial", 9), width=14)
        load_btn.pack(side=tk.LEFT, padx=2)
        
        # Boutons de configuration rapide
        quick_config_frame = tk.Frame(self.root, bg='#2b2b2b')
        quick_config_frame.pack(pady=(5, 10), fill=tk.X, padx=10)  # Plus d'espace en bas
        
        save_default_btn = tk.Button(quick_config_frame, text="💾 Défaut",
                                    command=self.save_default_config,
                                    bg='#3d6b3d', fg='white',
                                    font=("Arial", 8), width=10)
        save_default_btn.pack(side=tk.LEFT, padx=2)
        
        load_default_btn = tk.Button(quick_config_frame, text="📂 Défaut",
                                    command=self.load_default_config,
                                    bg='#3d6b3d', fg='white',
                                    font=("Arial", 8), width=10)
        load_default_btn.pack(side=tk.LEFT, padx=2)
        
        config_folder_btn = tk.Button(quick_config_frame, text="📁 Dossier",
                                     command=self.open_config_folder,
                                     bg='#3d6b3d', fg='white',
                                     font=("Arial", 8), width=10)
        config_folder_btn.pack(side=tk.LEFT, padx=2)
        
        # Mettre à jour les ports MIDI
        self.update_midi_ports()
        
    def create_tab_sliders(self, parent, tab_index):
        """Créer les 6 sliders pour un onglet"""
        sliders_frame = tk.Frame(parent, bg='#2b2b2b')
        sliders_frame.pack(pady=10, fill=tk.BOTH, expand=True, padx=10)
        
        # Frame pour centrer les sliders
        center_frame = tk.Frame(sliders_frame, bg='#2b2b2b')
        center_frame.pack(expand=True)
        
        for i in range(self.sliders_per_tab):
            self.create_slider(center_frame, tab_index, i)
    
    def create_program_change_tab(self):
        """Créer l'onglet Program Change"""
        pc_tab = tk.Frame(self.notebook, bg='#2b2b2b')
        self.notebook.add(pc_tab, text="Program Change")
        
        # Titre
        title = tk.Label(pc_tab, text="Program Change", 
                        font=("Arial", 14, "bold"), 
                        bg='#2b2b2b', fg='white')
        title.pack(pady=2)
        
        # Frame principal
        main_frame = tk.Frame(pc_tab, bg='#2b2b2b')
        main_frame.pack(pady=20, fill=tk.BOTH, expand=True)
        
        # Sélection du programme
        prog_frame = tk.Frame(main_frame, bg='#2b2b2b')
        prog_frame.pack(pady=15)
        
        tk.Label(prog_frame, text="Numéro de Programme:", 
                font=("Arial", 11), bg='#2b2b2b', fg='white').pack(side=tk.LEFT, padx=10)
        
        self.program_var = tk.StringVar(value="0")
        program_spin = tk.Spinbox(prog_frame, from_=0, to=127, textvariable=self.program_var,
                                 width=8, font=("Arial", 12),
                                 bg='#3b3b3b', fg='white', justify='center',
                                 buttonbackground='#4b4b4b')
        program_spin.pack(side=tk.LEFT, padx=10)
               
        # Canal de destination
        channel_frame = tk.Frame(main_frame, bg='#2b2b2b')
        channel_frame.pack(pady=15)
                      
        # Bouton d'envoi
        send_frame = tk.Frame(main_frame, bg='#2b2b2b')
        send_frame.pack(pady=25)
        
        send_btn = tk.Button(send_frame, text="📤 Envoyer Program Change",
                            command=self.send_program_change,
                            bg='#2d5c2d', fg='white',
                            font=("Arial", 12, "bold"),
                            width=25, height=2)
        send_btn.pack()
        
        # Presets rapides
        presets_frame = tk.LabelFrame(main_frame, text="Presets Rapides", 
                                     font=("Arial", 10, "bold"),
                                     bg='#2b2b2b', fg='white',
                                     labelanchor='n')
        presets_frame.pack(pady=20, padx=20, fill=tk.X)
        
        presets_grid = tk.Frame(presets_frame, bg='#2b2b2b')
        presets_grid.pack(pady=10)
        
        # Créer des boutons de presets
        presets = [
            ("1", 0), ("2", 1), ("3", 2), ("4", 3), ("5", 4),
            ("6", 5), ("7", 6), ("8", 7), ("9", 8), ("10", 9),
        ]
        for i, (name, prog) in enumerate(presets):
            row = i // 5
            col = i % 5
            btn = tk.Button(presets_grid, text=f"{name}",
                           command=lambda p=prog: self.set_program(p),
                           bg='#3b3b3b', fg='white',
                           font=("Arial", 12),
                           width=5, height=2)
            btn.grid(row=row, column=col, padx=3, pady=3)
        
        # Informations
        info_frame = tk.Frame(main_frame, bg='#2b2b2b')
        info_frame.pack(pady=20)
           
    def sync_program_vars(self):
        """Synchroniser les variables de programme"""
        try:
            val = int(self.program_entry_var.get())
            if 0 <= val <= 127:
                self.program_var.set(str(val))
        except ValueError:
            pass
    
    def sync_program_entry(self):
        """Synchroniser l'entry depuis le spinbox"""
        try:
            val = int(self.program_entry_var.get())
            if 0 <= val <= 127:
                # Ne pas créer de boucle
                if self.program_var.get() != str(val):
                    self.program_var.set(str(val))
        except ValueError:
            pass
    
    def toggle_pc_channel(self):
        """Basculer entre canal principal et canal personnalisé"""
        if self.pc_use_main_channel.get():
            self.pc_channel_spin.config(state='disabled')
            self.pc_omni_checkbutton.config(state='disabled')
        else:
            self.pc_channel_spin.config(state='normal')
            self.pc_omni_checkbutton.config(state='normal')
    
    def set_program(self, program):
        """Définir le numéro de programme"""
        self.program_var.set(str(program))
        self.program_entry_var.set(str(program))
    
    def send_program_change(self):
        """Envoyer un message Program Change"""
        if not self.midi_output:
            messagebox.showerror("Erreur", "Aucune sortie MIDI configurée!")
            return
        
        try:
            program = int(self.program_var.get())
            if not (0 <= program <= 127):
                messagebox.showerror("Erreur", "Le numéro de programme doit être entre 0 et 127")
                return
            
            # Déterminer les canaux à utiliser
            if self.pc_use_main_channel.get():
                # Utiliser la configuration principale
                if self.omni_mode:
                    channels = range(16)
                    mode_text = "tous canaux (Omni principal)"
                else:
                    channels = [self.midi_channel]
                    mode_text = f"canal {self.midi_channel + 1} (principal)"
            else:
                # Utiliser la configuration personnalisée
                if self.pc_omni_var.get():
                    channels = range(16)
                    mode_text = "tous canaux (Omni PC)"
                else:
                    pc_channel = int(self.pc_channel_var.get()) - 1
                    channels = [pc_channel]
                    mode_text = f"canal {pc_channel + 1} (spécifique)"
            
            # Envoyer les messages
            for channel in channels:
                msg = mido.Message('program_change', channel=channel, program=program)
                self.midi_output.send(msg)
            
            print(f"Program Change {program} envoyé sur {mode_text}")
            messagebox.showinfo("Succès", f"Program Change {program}\nenvoyé sur {mode_text}")
            
        except ValueError:
            messagebox.showerror("Erreur", "Numéro de programme invalide")
        except Exception as e:
            messagebox.showerror("Erreur", f"Erreur MIDI: {e}")
    
    def create_slider(self, parent, tab_index, slider_index):
        """Créer un slider avec son contrôle CC"""
        slider_frame = tk.Frame(parent, bg='#2b2b2b')
        slider_frame.pack(pady=8, fill=tk.X)
        
        # Label du slider
        slider_label = tk.Label(slider_frame, text=f"S{slider_index + 1}:", 
                               font=("Arial", 9), bg='#2b2b2b', fg='white', width=3)
        slider_label.grid(row=0, column=0, sticky='w')
        
        # Contrôle CC
        cc_frame = tk.Frame(slider_frame, bg='#2b2b2b')
        cc_frame.grid(row=0, column=1, padx=5)
        
        tk.Label(cc_frame, text="CC:", 
                font=("Arial", 8), bg='#2b2b2b', fg='white').pack(side=tk.LEFT)
        
        cc_var = tk.StringVar(value=str(self.slider_cc[tab_index][slider_index]))
        cc_entry = tk.Entry(cc_frame, textvariable=cc_var, width=3,
                           bg='#3b3b3b', fg='white', justify='center',
                           insertbackground='white')
        cc_entry.pack(side=tk.LEFT, padx=2)
        cc_entry.bind('<Return>', lambda e, t=tab_index, s=slider_index: self.update_cc_number(t, s))
        cc_entry.bind('<FocusOut>', lambda e, t=tab_index, s=slider_index: self.update_cc_number(t, s))
        
        self.cc_entries[tab_index].append(cc_var)
        
        # Saisie valeur
        value_var = tk.StringVar(value="0")
        value_entry = tk.Entry(slider_frame, textvariable=value_var, width=4,
                              bg='#3b3b3b', fg='#00ff00', justify='center',
                              insertbackground='white', font=("Arial", 9, "bold"))
        value_entry.grid(row=0, column=2, padx=5)
        value_entry.bind('<Return>', lambda e, t=tab_index, s=slider_index: self.update_value_from_entry(t, s))
        value_entry.bind('<FocusOut>', lambda e, t=tab_index, s=slider_index: self.update_value_from_entry(t, s))
        
        self.value_labels[tab_index].append(value_var)
        
        # Slider centré
        slider = tk.Scale(slider_frame, from_=0, to=127, 
                         orient=tk.HORIZONTAL, length=250,
                         command=lambda value, t=tab_index, s=slider_index: self.on_slider_change(value, t, s),
                         bg='#3b3b3b', fg='white', 
                         highlightthickness=0, troughcolor='#1b1b1b',
                         sliderrelief='raised')
        slider.grid(row=1, column=0, columnspan=3, sticky='ew', pady=2)
        slider.set(0)
        
        self.sliders[tab_index].append(slider)
        
        slider_frame.columnconfigure(0, weight=0)
        slider_frame.columnconfigure(1, weight=0)
        slider_frame.columnconfigure(2, weight=0)
    
    def update_cc_number(self, tab_index, slider_index):
        """Mettre à jour le numéro CC pour un slider"""
        try:
            new_cc = int(self.cc_entries[tab_index][slider_index].get())
            if 0 <= new_cc <= 127:
                self.slider_cc[tab_index][slider_index] = new_cc
                print(f"Onglet {tab_index + 1}, Slider {slider_index + 1} CC -> {new_cc}")
            else:
                self.cc_entries[tab_index][slider_index].set(str(self.slider_cc[tab_index][slider_index]))
                print("CC doit être entre 0 et 127")
        except ValueError:
            self.cc_entries[tab_index][slider_index].set(str(self.slider_cc[tab_index][slider_index]))
            print("CC doit être un nombre valide")
    
    def on_slider_change(self, value, tab_index, slider_index):
        """Gérer le changement d'un slider"""
        self.slider_values[tab_index][slider_index] = int(float(value))
        self.value_labels[tab_index][slider_index].set(str(self.slider_values[tab_index][slider_index]))
        self.send_midi(tab_index, slider_index)
    
    def update_value_from_entry(self, tab_index, slider_index):
        """Mettre à jour le slider depuis la saisie texte"""
        try:
            new_value = int(self.value_labels[tab_index][slider_index].get())
            if 0 <= new_value <= 127:
                self.sliders[tab_index][slider_index].set(new_value)
                self.slider_values[tab_index][slider_index] = new_value
                self.send_midi(tab_index, slider_index)
            else:
                self.value_labels[tab_index][slider_index].set(str(self.slider_values[tab_index][slider_index]))
                print("Valeur doit être entre 0 et 127")
        except ValueError:
            self.value_labels[tab_index][slider_index].set(str(self.slider_values[tab_index][slider_index]))
            print("Valeur doit être un nombre valide")
    
    def send_midi(self, tab_index, slider_index):
        """Envoyer un message MIDI pour un slider spécifique"""
        if self.midi_output:
            try:
                value = self.slider_values[tab_index][slider_index]
                cc = self.slider_cc[tab_index][slider_index]
                
                if self.omni_mode:
                    for channel in range(16):
                        msg = mido.Message('control_change', 
                                          channel=channel,
                                          control=cc,
                                          value=value)
                        self.midi_output.send(msg)
                    print(f"Tab {tab_index + 1}, S{slider_index + 1} -> CC{cc} = {value} (Omni)")
                else:
                    msg = mido.Message('control_change', 
                                      channel=self.midi_channel,
                                      control=cc,
                                      value=value)
                    self.midi_output.send(msg)
                    print(f"Tab {tab_index + 1}, S{slider_index + 1} -> CC{cc} = {value} (Ch {self.midi_channel + 1})")
            except Exception as e:
                print(f"Erreur MIDI: {e}")
    
    def test_current_tab(self):
        """Tester tous les sliders de l'onglet actuel"""
        current_tab = self.notebook.index(self.notebook.select())
        # Vérifier que ce n'est pas l'onglet Program Change
        if current_tab >= self.num_tabs:
            print("L'onglet Program Change n'a pas de sliders à tester")
            return
            
        if self.midi_output:
            print(f"Test onglet {current_tab + 1}")
            for i in range(self.sliders_per_tab):
                self.send_midi(current_tab, i)
        else:
            print("Aucune sortie MIDI configurée")
    
    def reset_current_tab(self):
        """Remettre à zéro tous les sliders de l'onglet actuel"""
        current_tab = self.notebook.index(self.notebook.select())
        # Vérifier que ce n'est pas l'onglet Program Change
        if current_tab >= self.num_tabs:
            print("L'onglet Program Change ne peut pas être réinitialisé")
            return
            
        for i in range(self.sliders_per_tab):
            self.sliders[current_tab][i].set(0)
            self.slider_values[current_tab][i] = 0
            self.value_labels[current_tab][i].set("0")
            self.send_midi(current_tab, i)
        print(f"Onglet {current_tab + 1} remis à zéro")
    
    def test_all_sliders(self):
        """Tester tous les sliders de tous les onglets"""
        if self.midi_output:
            print("Test de tous les onglets")
            for tab in range(self.num_tabs):
                for slider in range(self.sliders_per_tab):
                    self.send_midi(tab, slider)
        else:
            print("Aucune sortie MIDI configurée")
    
    def reset_all_sliders(self):
        """Remettre à zéro tous les sliders de tous les onglets"""
        for tab in range(self.num_tabs):
            for slider in range(self.sliders_per_tab):
                self.sliders[tab][slider].set(0)
                self.slider_values[tab][slider] = 0
                # CORRECTION : Utiliser set() au lieu de config() pour StringVar
                self.value_labels[tab][slider].set("0")
                self.send_midi(tab, slider)
        print("Tous les onglets remis à zéro")
    
    def toggle_omni_mode(self):
        """Activer/désactiver le mode canal de diffusion"""
        self.omni_mode = self.omni_var.get()
        if self.omni_mode:
            print("Mode Omni ACTIVÉ - Envoi sur tous les canaux")
        else:
            print(f"Mode Omni DÉSACTIVÉ - Canal {self.midi_channel + 1}")
    
    def get_config_data(self):
        """Obtenir les données de configuration actuelles"""
        config = {
            'slider_values': self.slider_values,
            'slider_cc': self.slider_cc,
            'midi_channel': self.midi_channel,
            'omni_mode': self.omni_mode,
            'interface': self.interface_var.get(),
            'port': self.port_var.get()
        }
        return config
    
    def apply_config(self, config):
        """Appliquer une configuration chargée"""
        try:
            for tab in range(min(self.num_tabs, len(config['slider_values']))):
                for slider in range(min(self.sliders_per_tab, len(config['slider_values'][tab]))):
                    value = config['slider_values'][tab][slider]
                    self.sliders[tab][slider].set(value)
                    self.slider_values[tab][slider] = value
                    # CORRECTION : Utiliser set() au lieu de config() pour StringVar
                    self.value_labels[tab][slider].set(str(value))
            
            for tab in range(min(self.num_tabs, len(config['slider_cc']))):
                for slider in range(min(self.sliders_per_tab, len(config['slider_cc'][tab]))):
                    cc = config['slider_cc'][tab][slider]
                    self.slider_cc[tab][slider] = cc
                    self.cc_entries[tab][slider].set(str(cc))
            
            if 'midi_channel' in config:
                self.midi_channel = config['midi_channel']
                self.channel_var.set(str(self.midi_channel + 1))
            
            if 'omni_mode' in config:
                self.omni_mode = config['omni_mode']
                self.omni_var.set(self.omni_mode)
            
            if 'interface' in config and config['interface'] in self.midi_interfaces:
                self.interface_var.set(config['interface'])
                self.update_midi_ports()
                
                if 'port' in config and config['port'] in self.midi_ports:
                    self.port_var.set(config['port'])
                    self.change_midi_port()
            
            print("Configuration appliquée")
            return True
            
        except Exception as e:
            print(f"Erreur: {e}")
            return False
        
    def save_config(self, filename):
        """Sauvegarder la configuration"""
        try:
            config = self.get_config_data()
            with open(filename, 'w') as f:
                json.dump(config, f, indent=4)
            print(f"Configuration sauvegardée: {filename}")
            return True
        except Exception as e:
            print(f"Erreur de sauvegarde: {e}")
            return False
    
    def load_config(self, filename):
        """Charger la configuration"""
        try:
            with open(filename, 'r') as f:
                config = json.load(f)
            
            if self.apply_config(config):
                print(f"Configuration chargée: {filename}")
                return True
            else:
                return False
                
        except Exception as e:
            print(f"Erreur de chargement: {e}")
            return False
    
    def save_config_dialog(self):
        """Dialogue de sauvegarde"""
        filename = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("Fichiers JSON", "*.json"), ("Tous les fichiers", "*.*")],
            title="Sauvegarder la configuration",
            initialdir=self.config_dir
        )
        if filename:
            if self.save_config(filename):
                messagebox.showinfo("Succès", f"Configuration sauvegardée")
            else:
                messagebox.showerror("Erreur", "Erreur lors de la sauvegarde")
    
    def load_config_dialog(self):
        """Dialogue de chargement"""
        filename = filedialog.askopenfilename(
            defaultextension=".json",
            filetypes=[("Fichiers JSON", "*.json"), ("Tous les fichiers", "*.*")],
            title="Charger la configuration",
            initialdir=self.config_dir
        )
        if filename:
            if self.load_config(filename):
                messagebox.showinfo("Succès", f"Configuration chargée")
            else:
                messagebox.showerror("Erreur", "Erreur lors du chargement")
    
    def save_default_config(self):
        """Sauvegarder la configuration par défaut"""
        if self.save_config(self.default_config_file):
            messagebox.showinfo("Succès", "Configuration par défaut sauvegardée")
        else:
            messagebox.showerror("Erreur", "Erreur de sauvegarde")
    
    def load_default_config(self):
        """Charger la configuration par défaut"""
        if os.path.exists(self.default_config_file):
            if self.load_config(self.default_config_file):
                print("Configuration par défaut chargée")
    
    def open_config_folder(self):
        """Ouvrir le dossier de configuration"""
        try:
            if os.path.exists(self.config_dir):
                os.startfile(self.config_dir)
            else:
                messagebox.showinfo("Info", "Le dossier n'existe pas encore.")
        except:
            try:
                import subprocess
                subprocess.call(['open', self.config_dir])
            except:
                messagebox.showinfo("Dossier", f"{self.config_dir}")
    
    def get_midi_interfaces(self):
        """Obtenir les interfaces MIDI"""
        interfaces = []
        try:
            all_ports = mido.get_output_names()
            interface_names = set()
            for port in all_ports:
                if ':' in port:
                    interface_name = port.split(':')[0]
                else:
                    interface_name = port
                interface_names.add(interface_name)
            
            interfaces = sorted(list(interface_names))
            
        except Exception as e:
            print(f"Erreur MIDI: {e}")
        
        return interfaces if interfaces else ["Aucune interface disponible"]
    
    def update_midi_ports(self):
        """Mettre à jour les ports MIDI"""
        try:
            all_ports = mido.get_output_names()
            selected_interface = self.interface_var.get()
            
            filtered_ports = []
            for port in all_ports:
                if selected_interface in port:
                    filtered_ports.append(port)
            
            self.midi_ports = filtered_ports if filtered_ports else ["Aucun port disponible"]
            self.port_combo['values'] = self.midi_ports
            
            if self.midi_ports:
                self.port_var.set(self.midi_ports[0])
            else:
                self.port_var.set("Aucun port disponible")
                
            self.change_midi_port()
            
        except Exception as e:
            print(f"Erreur: {e}")
            self.midi_ports = ["Aucun port disponible"]
            self.port_combo['values'] = self.midi_ports
            self.port_var.set(self.midi_ports[0])
    
    def change_midi_interface(self, event=None):
        """Changer l'interface MIDI"""
        self.update_midi_ports()
    
    def setup_midi(self):
        """Configurer la sortie MIDI"""
        try:
            if self.midi_ports and self.midi_ports[0] != "Aucun port disponible":
                self.midi_output = mido.open_output(self.port_var.get())
                print(f"Connecté: {self.port_var.get()}")
        except Exception as e:
            print(f"Erreur MIDI: {e}")
            self.midi_output = None
    
    def change_midi_port(self, event=None):
        """Changer le port MIDI"""
        if self.midi_output:
            self.midi_output.close()
        self.setup_midi()
    
    def refresh_midi(self):
        """Rafraîchir MIDI"""
        self.midi_interfaces = self.get_midi_interfaces()
        self.interface_combo['values'] = self.midi_interfaces
        if self.midi_interfaces:
            self.interface_var.set(self.midi_interfaces[0])
        
        self.update_midi_ports()
    
    def update_midi_channel(self):
        """Mettre à jour le canal MIDI"""
        try:
            self.midi_channel = int(self.channel_var.get()) - 1
            print(f"Canal MIDI: {self.midi_channel + 1}")
        except ValueError:
            self.midi_channel = 0
    
    def cleanup(self):
        """Nettoyer les ressources"""
        if self.midi_output:
            self.midi_output.close()

def main():
    root = tk.Tk()
    app = PotentiometerMIDI(root)
    
    def on_closing():
        app.cleanup()
        root.destroy()
    
    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()

if __name__ == "__main__":
    main()