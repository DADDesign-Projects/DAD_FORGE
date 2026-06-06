#****************************************************************************
# Color Configurator
#
# Description:
# Utility to configure the colors used by GUI_PENDA
# in the FORGE library project used to build audio effect pedals.
#
# Thanks to Qwent-Coder
# Copyright (c) 2025 Dad Design.
#****************************************************************************
import tkinter as tk
from tkinter import colorchooser, messagebox, filedialog
import json
import os
import serial
import serial.tools.list_ports

# List of parameters
parameters = [
    "MenuBack", "MenuText", "MenuActiveText", "MenuActive", "MenuArrow",
    "ParameterBack", "ParameterName", "ParameterNum", "ParameterValue", "ParameterLines", "ParameterCursor",
    "ParamInfoBack", "ParamInfoName", "ParamInfoValue",
    "MainInfoBack", "MainInfoMem", "MainInfoDirty", "MainInfoState",
    "MemViewBack", "MemViewText", "MemViewLine", "MemViewActive",
    "VuMeterBack", "VuMeterText", "VuMeterLine", "VuMeterCursor", "VuMeterPeak", "VuMeterClip",
    "SplatchBack", "SplatchText"
]

class ColorConfigApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Color Configurator")
        self.root.configure(bg='#f0f0f0')  # Window background color
        self.colors = {param: "#FFFFFF" for param in parameters}
        self.current_file = None
        self.serial_port = None

        self.create_widgets()
    
    def create_widgets(self):
        # Main frame for cleaner layout
        main_frame = tk.Frame(self.root, bg='#f0f0f0', padx=10, pady=10)
        main_frame.pack(fill='both', expand=True)

        # Frame for color parameters
        color_frame = tk.LabelFrame(main_frame, text="Color Parameters", bg='#f0f0f0', font=('Arial', 10, 'bold'))
        color_frame.pack(side='left', fill='both', expand=True, padx=(0, 5), pady=5)

        # Canvas and Scrollbar for colors (if the list is long)
        canvas_frame = tk.Frame(color_frame)
        canvas_frame.pack(fill='both', expand=True)

        self.canvas = tk.Canvas(canvas_frame, bg='#f0f0f0')
        scrollbar = tk.Scrollbar(canvas_frame, orient="vertical", command=self.canvas.yview)
        scrollable_frame = tk.Frame(self.canvas, bg='#f0f0f0')

        scrollable_frame.bind(
            "<Configure>",
            lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all"))
        )

        self.canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        self.canvas.configure(yscrollcommand=scrollbar.set)

        # Add parameters to the scrollable frame
        row = 0
        for param in parameters:
            label = tk.Label(scrollable_frame, text=param, width=20, anchor='w', bg='#f0f0f0')
            label.grid(row=row, column=0, padx=5, pady=2, sticky='w')

            color_frame_param = tk.Frame(scrollable_frame, bg=self.colors[param], width=30, height=20, relief='sunken', bd=1)
            color_frame_param.grid(row=row, column=1, padx=5, pady=2)
            setattr(self, f"{param}_frame", color_frame_param)

            button = tk.Button(
                scrollable_frame,
                text="Choose",
                command=lambda p=param: self.choose_color(p)
            )
            button.grid(row=row, column=2, padx=5, pady=2)
            setattr(self, f"{param}_button", button) # For potential style updates

            row += 1

        # Add canvas and scrollbar to the main frame
        self.canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Frame for controls (right side)
        control_frame = tk.Frame(main_frame, bg='#f0f0f0')
        control_frame.pack(side='right', fill='y', padx=(5, 0), pady=5)

        # COM Port Section
        com_frame = tk.LabelFrame(control_frame, text="COM Port", bg='#f0f0f0', font=('Arial', 10, 'bold'))
        com_frame.pack(fill='x', pady=5)

        tk.Label(com_frame, text="COM Port:", bg='#f0f0f0').pack(anchor='w', padx=5)
        self.port_var = tk.StringVar(value="COM1")
        port_list = [port.device for port in serial.tools.list_ports.comports()]
        self.port_menu = tk.OptionMenu(com_frame, self.port_var, *port_list)
        self.port_menu.config(bg='white', relief='sunken', bd=1)
        self.port_menu.pack(fill='x', padx=5, pady=2)

        open_btn = tk.Button(com_frame, text="Open Port", command=self.open_serial_port, bg='#4CAF50', fg='white', relief='raised')
        open_btn.pack(fill='x', padx=5, pady=5)

        # File Section
        file_frame = tk.LabelFrame(control_frame, text="File Operations", bg='#f0f0f0', font=('Arial', 10, 'bold'))
        file_frame.pack(fill='x', pady=5)

        save_btn = tk.Button(file_frame, text="Save Colors As...", command=self.save_colors_to_file, bg='#2196F3', fg='white', relief='raised')
        save_btn.pack(fill='x', padx=5, pady=2)

        load_btn = tk.Button(file_frame, text="Load Colors From...", command=self.load_colors_from_file, bg='#2196F3', fg='white', relief='raised')
        load_btn.pack(fill='x', padx=5, pady=2)

        save_cpp_btn = tk.Button(file_frame, text="Save C++ Colors As...", command=self.save_cpp_colors, bg='#FF9800', fg='white', relief='raised')
        save_cpp_btn.pack(fill='x', padx=5, pady=2)

        # COM Transmission Section
        send_frame = tk.LabelFrame(control_frame, text="COM Transmission", bg='#f0f0f0', font=('Arial', 10, 'bold'))
        send_frame.pack(fill='x', pady=5)

        send_btn = tk.Button(send_frame, text="Send Colors via COM", command=self.send_colors_via_com, bg='#F44336', fg='white', relief='raised')
        send_btn.pack(fill='x', padx=5, pady=5)


    def choose_color(self, param):
        # Initialize dialog with current color
        initial_color = self.colors.get(param, "#FFFFFF")
        color_code = colorchooser.askcolor(
            title=f"Choose color for {param}",
            color=initial_color
        )[1]
        if color_code:
            self.colors[param] = color_code
            frame = getattr(self, f"{param}_frame")
            frame.config(bg=color_code)

    def save_colors_to_file(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        if file_path:
            with open(file_path, 'w') as f:
                json.dump(self.colors, f, indent=4)
            self.current_file = file_path
            #messagebox.showinfo("Saved", f"Colors saved to:\n{file_path}")

    def load_colors_from_file(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        if file_path and os.path.exists(file_path):
            with open(file_path, 'r') as f:
                self.colors = json.load(f)
            self.current_file = file_path
            # Update the interface
            for param in parameters:
                frame = getattr(self, f"{param}_frame")
                frame.config(bg=self.colors.get(param, "#FFFFFF"))
            #messagebox.showinfo("Loaded", f"Colors loaded from:\n{file_path}")

    def save_cpp_colors(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".hpp",
            filetypes=[("Header files", "*.hpp"), ("All files", "*.*")]
        )
        if not file_path:
            return

        try:
            with open(file_path, 'w') as f:
                f.write("\t{\n")
                for i, param in enumerate(parameters):
                    hex_color = self.colors.get(param, "#FFFFFF")
                    rgb = self.hex_to_rgb(hex_color)
                    param_name_for_comment = param
                    if param == "VuMeterPeak":
                        param_name_for_comment = "VuMeterPick"
                    
                    comma = "," if i < len(parameters) - 1 else ""
                    f.write(f"\t\tDadGFX::sColor({rgb[0]}, {rgb[1]}, {rgb[2]}, 255){comma}\t// \t{param_name_for_comment}\n")

                f.write("\t}\n")
            
            messagebox.showinfo("Saved", f"C++ color palette saved to:\n{file_path}")
        except Exception as e:
            messagebox.showerror("Error", f"Could not save file:\n{str(e)}")

    def hex_to_rgb(self, hex_color):
        """Converts a hexadecimal color to an RGB tuple."""
        hex_color = hex_color.lstrip('#')
        return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))

    def open_serial_port(self):
        port = self.port_var.get()
        try:
            self.serial_port = serial.Serial(port, 9600, timeout=1)
            messagebox.showinfo("Success", f"Port {port} opened.")
        except Exception as e:
            messagebox.showerror("Error", f"Could not open {port}:\n{str(e)}")

    def send_colors_via_com(self):
        if not self.serial_port or not self.serial_port.is_open:
            messagebox.showwarning("Warning", "Please open a COM port first.")
            return

        for param, color in self.colors.items():
            message = f"{param}={color}\n"
            self.serial_port.write(message.encode('utf-8'))

        messagebox.showinfo("Success", "Colors sent via COM port.")

root = tk.Tk()
app = ColorConfigApp(root)
root.mainloop()