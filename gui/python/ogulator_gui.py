#!/usr/bin/env python3
"""
OGULATOR - Modern Python GUI Calculator with C++ Engine
Simplified version with progressive package loading
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import subprocess
import threading
import queue
import os
import sys
from pathlib import Path
import tempfile
import time

class CppEngineInterface:
    """🏎️ ULTRA-FAST C++ calculator engine - TRUE Senna speed! 🏎️"""
    
    def __init__(self, executable_path):
        self.executable_path = executable_path
    
    def execute_command(self, command):
        """🏎️ ULTRA-FAST C++ execution - TRUE Senna speed at Monaco! 🏎️"""
        if not self.executable_path:
            return {
                'success': False,
                'error': 'C++ engine not available',
                'fallback_needed': True
            }
        
        try:
            # 🏎️ SENNA SPEED - Command-line batch mode! ⚡
            start_time = time.time()
            
            result = subprocess.run(
                [self.executable_path, command],  # Pass command as argument!
                capture_output=True,
                text=True,
                timeout=3.0,  # Still need timeout for safety
                encoding='utf-8',
                errors='ignore',
                creationflags=subprocess.CREATE_NO_WINDOW if sys.platform == 'win32' else 0,
                startupinfo=subprocess.STARTUPINFO(dwFlags=subprocess.STARTF_USESHOWWINDOW, wShowWindow=subprocess.SW_HIDE) if sys.platform == 'win32' else None
            )
            
            execution_time = (time.time() - start_time) * 1000  # Convert to ms
            
            if result.returncode == 0:
                # Parse the direct numerical output
                result_text = result.stdout.strip()
                
                # Filter out Python loading messages
                lines = result_text.split('\n')
                clean_lines = [line.strip() for line in lines 
                             if line.strip() and 
                             not any(msg in line for msg in ['loaded successfully', 'NumPy', 'SciPy', 'Matplotlib', 'Pandas', 'SymPy'])]
                
                if clean_lines:
                    final_result = clean_lines[-1]  # Get the actual calculation result
                    
                    return {
                        'success': True,
                        'result': final_result,
                        'execution_time': round(execution_time, 1),
                        'senna_speed': execution_time < 100,  # Under 100ms = Senna speed!
                        'f1_speed': execution_time < 200      # Under 200ms = F1 speed!
                    }
                else:
                    return {
                        'success': True,
                        'result': 'Processed',
                        'execution_time': round(execution_time, 1)
                    }
            else:
                return {
                    'success': False,
                    'error': result.stderr.strip() if result.stderr else 'Execution failed',
                    'fallback_needed': True
                }
                
        except subprocess.TimeoutExpired:
            return {
                'success': False,
                'error': f'C++ engine timeout (3s) - using Python fallback',
                'fallback_needed': True
            }
        except Exception as e:
            return {
                'success': False,
                'error': str(e),
                'fallback_needed': True
            }

class OGULATORCalculatorGUI:
    """🏎️ SENNA SPEED OGULATOR Calculator GUI - Monaco GP Performance! 🏎️"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("🧮 OGULATOR - Advanced Scientific Calculator")
        self.root.geometry("1200x800")
        self.root.minsize(800, 600)
        
        # Initialize C++ engine interface
        self.cpp_engine = None
        self.engine_available = self.init_cpp_engine()
        
        # Calculator state
        self.current_mode = "ALGEBRAIC"
        self.history = []
        self.python_repl_mode = False
        self.python_globals = {}
        
        # Available packages (loaded progressively)
        self.available_packages = self.check_packages()
        
        # Setup GUI
        self.setup_theme()
        self.create_interface()
        self.setup_bindings()
        self.add_welcome_message()
    
    def check_packages(self):
        """Check which scientific packages are available"""
        packages = {}
        
        try:
            import numpy
            packages['numpy'] = True
        except ImportError:
            packages['numpy'] = False
        
        try:
            import scipy
            packages['scipy'] = True
        except ImportError:
            packages['scipy'] = False
        
        try:
            import matplotlib
            packages['matplotlib'] = True
        except ImportError:
            packages['matplotlib'] = False
        
        try:
            import pandas
            packages['pandas'] = True
        except ImportError:
            packages['pandas'] = False
        
        try:
            import sympy
            packages['sympy'] = True
        except ImportError:
            packages['sympy'] = False
        
        return packages
    
    def init_cpp_engine(self):
        """Initialize the C++ engine interface"""
        executable_path = self.find_cpp_executable()
        if executable_path:
            self.cpp_engine = CppEngineInterface(executable_path)
            return True
        return False
    
    def find_cpp_executable(self):
        """Find the C++ executable"""
        current_dir = Path(__file__).parent
        possible_paths = [
            current_dir / "build-ninja" / "cpp_dynamic_calc.exe",
            current_dir / "build" / "cpp_dynamic_calc.exe",
            current_dir / "build-ninja" / "Debug" / "cpp_dynamic_calc.exe",
            current_dir / "build" / "Debug" / "cpp_dynamic_calc.exe",
            current_dir / "cmake-build-debug" / "cpp_dynamic_calc.exe",
            current_dir / "build-ninja" / "cpp_dynamic_calc",
            current_dir / "build" / "cpp_dynamic_calc",
        ]
        
        for path in possible_paths:
            if path.exists():
                return str(path)
        return None
    
    def setup_theme(self):
        """Setup modern theme"""
        style = ttk.Style()
        
        # Use a modern theme
        try:
            style.theme_use('vista' if sys.platform.startswith('win') else 'clam')
        except:
            style.theme_use('default')
        
        # Define colors
        self.colors = {
            'bg_dark': '#2b2b2b',
            'bg_medium': '#3c3c3c',
            'bg_light': '#4d4d4d',
            'fg_primary': '#ffffff',
            'fg_secondary': '#cccccc',
            'accent_blue': '#0078d4',
            'accent_green': '#16c60c',
            'accent_red': '#e74856',
            'accent_orange': '#ff8c00',
            'accent_purple': '#8764b8'
        }
        
        # Configure root window
        self.root.configure(bg=self.colors['bg_dark'])
    
    def create_interface(self):
        """Create the main interface"""
        # Main container
        main_container = ttk.Frame(self.root)
        main_container.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Header
        self.create_header(main_container)
        
        # Main content area
        content_frame = ttk.Frame(main_container)
        content_frame.pack(fill=tk.BOTH, expand=True, pady=(10, 0))
        
        # Create paned window for resizable layout
        paned = ttk.PanedWindow(content_frame, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True)
        
        # Left panel - Controls
        left_panel = self.create_left_panel(paned)
        paned.add(left_panel, weight=1)
        
        # Right panel - IO
        right_panel = self.create_right_panel(paned)
        paned.add(right_panel, weight=2)
        
        # Status bar
        self.create_status_bar(main_container)
    
    def create_header(self, parent):
        """Create header with title and status"""
        header_frame = ttk.Frame(parent)
        header_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Title
        title_label = ttk.Label(header_frame, text="🧮 OGULATOR", 
                               font=('Segoe UI', 20, 'bold'))
        title_label.pack(side=tk.LEFT)
        
        # Status area
        status_area = ttk.Frame(header_frame)
        status_area.pack(side=tk.RIGHT)
        
        self.mode_label = ttk.Label(status_area, text=f"Mode: {self.current_mode}",
                                   font=('Segoe UI', 11, 'bold'),
                                   foreground=self.colors['accent_blue'])
        self.mode_label.pack(anchor=tk.E)
        
        engine_status = "🟢 C++ Engine Ready" if self.engine_available else "🟡 Python Fallback"
        engine_color = self.colors['accent_green'] if self.engine_available else self.colors['accent_orange']
        self.engine_label = ttk.Label(status_area, text=engine_status,
                                     foreground=engine_color)
        self.engine_label.pack(anchor=tk.E)
        
        # Package status
        available_count = sum(self.available_packages.values())
        total_count = len(self.available_packages)
        pkg_status = f"📦 Packages: {available_count}/{total_count}"
        self.package_label = ttk.Label(status_area, text=pkg_status,
                                      foreground=self.colors['fg_secondary'])
        self.package_label.pack(anchor=tk.E)
    
    def create_left_panel(self, parent):
        """Create left control panel"""
        left_frame = ttk.Frame(parent)
        
        # Mode selection
        mode_frame = ttk.LabelFrame(left_frame, text=" Calculator Modes ")
        mode_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Basic modes
        basic_modes = [
            ("📊 Algebraic", "algebraic", "Basic mathematics"),
            ("📈 Linear System", "linear", "Matrix & system solving"),
            ("📊 Statistics", "stats", "Statistical analysis"),
            ("∫ Symbolic", "symbolic", "Symbolic computation"),
            ("📈 Plotting", "plot", "Function plotting"),
            ("📏 Units", "units", "Unit conversions")
        ]
        
        for i, (text, mode, desc) in enumerate(basic_modes):
            row = i // 2
            col = i % 2
            btn = ttk.Button(mode_frame, text=text, width=18,
                           command=lambda m=mode: self.change_mode(m))
            btn.grid(row=row, column=col, padx=2, pady=2, sticky='ew')
            # Simple tooltip via status bar
            btn.bind('<Enter>', lambda e, d=desc: self.set_status(d))
            btn.bind('<Leave>', lambda e: self.set_status("Ready"))
        
        # Configure grid weights
        mode_frame.columnconfigure(0, weight=1)
        mode_frame.columnconfigure(1, weight=1)
        
        # Python modes (only show if packages are available)
        python_frame = ttk.LabelFrame(left_frame, text=" Python Scientific Computing ")
        python_frame.pack(fill=tk.X, padx=5, pady=5)
        
        python_modes = [
            ("🐍 Python", "python", "Interactive Python", True),
            ("🔢 NumPy", "numpy", "Scientific arrays", self.available_packages.get('numpy', False)),
            ("⚗️ SciPy", "scipy", "Advanced math", self.available_packages.get('scipy', False)),
            ("📊 Matplotlib", "matplotlib", "Plotting", self.available_packages.get('matplotlib', False)),
            ("📋 Pandas", "pandas", "Data analysis", self.available_packages.get('pandas', False)),
            ("∑ SymPy", "sympy", "Symbolic math", self.available_packages.get('sympy', False))
        ]
        
        python_row = 0
        for text, mode, desc, available in python_modes:
            col = python_row % 2
            row = python_row // 2
            
            btn = ttk.Button(python_frame, text=text, width=18,
                           command=lambda m=mode: self.change_mode(m),
                           state='normal' if available else 'disabled')
            btn.grid(row=row, column=col, padx=2, pady=2, sticky='ew')
            
            # Add tooltip
            tooltip_text = desc if available else f"{desc} (Package not installed)"
            btn.bind('<Enter>', lambda e, d=tooltip_text: self.set_status(d))
            btn.bind('<Leave>', lambda e: self.set_status("Ready"))
            
            python_row += 1
        
        python_frame.columnconfigure(0, weight=1)
        python_frame.columnconfigure(1, weight=1)
        
        # Quick actions
        actions_frame = ttk.LabelFrame(left_frame, text=" Quick Actions ")
        actions_frame.pack(fill=tk.X, padx=5, pady=5)
        
        actions = [
            ("🔄 Clear Input", self.clear_input),
            ("🧹 Clear History", self.clear_history),
            ("💾 Save Results", self.save_results),
            ("📖 Help", self.show_help),
            ("🔧 Build Engine", self.try_build_engine),
            ("📦 Install Packages", self.install_packages_dialog)
        ]
        
        for i, (text, cmd) in enumerate(actions):
            row = i // 2
            col = i % 2
            btn = ttk.Button(actions_frame, text=text, width=18, command=cmd)
            btn.grid(row=row, column=col, padx=2, pady=2, sticky='ew')
        
        actions_frame.columnconfigure(0, weight=1)
        actions_frame.columnconfigure(1, weight=1)
        
        return left_frame
    
    def create_right_panel(self, parent):
        """Create right input/output panel"""
        right_frame = ttk.Frame(parent)
        
        # Input area
        input_frame = ttk.LabelFrame(right_frame, text=" Input ")
        input_frame.pack(fill=tk.X, padx=5, pady=(0, 5))
        
        self.input_text = scrolledtext.ScrolledText(
            input_frame,
            height=6,
            font=('Consolas', 11),
            wrap=tk.WORD
        )
        self.input_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Input controls
        input_controls = ttk.Frame(input_frame)
        input_controls.pack(fill=tk.X, padx=5, pady=(0, 5))
        
        execute_btn = ttk.Button(input_controls, text="▶️ Execute (Ctrl+Enter)",
                               command=self.execute_command)
        execute_btn.pack(side=tk.LEFT)
        
        clear_input_btn = ttk.Button(input_controls, text="🗑️ Clear Input",
                                   command=self.clear_input)
        clear_input_btn.pack(side=tk.LEFT, padx=(5, 0))
        
        # Mode indicator
        self.input_mode_label = ttk.Label(input_controls, text="Normal Mode",
                                         foreground=self.colors['accent_green'])
        self.input_mode_label.pack(side=tk.RIGHT)
        
        # Output area
        output_frame = ttk.LabelFrame(right_frame, text=" Results & History ")
        output_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=(5, 0))
        
        self.output_text = scrolledtext.ScrolledText(
            output_frame,
            font=('Consolas', 10),
            wrap=tk.WORD,
            state=tk.DISABLED
        )
        self.output_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Configure text tags
        self.setup_text_tags()
        
        return right_frame
    
    def create_status_bar(self, parent):
        """Create status bar"""
        self.status_frame = ttk.Frame(parent)
        self.status_frame.pack(fill=tk.X, pady=(5, 0))
        
        self.status_label = ttk.Label(self.status_frame, text="Ready")
        self.status_label.pack(side=tk.LEFT)
        
        self.progress_bar = ttk.Progressbar(self.status_frame, length=200,
                                          mode='indeterminate')
        self.progress_bar.pack(side=tk.RIGHT)
    
    def setup_text_tags(self):
        """Setup colored text tags for output"""
        tags = {
            "input": {"foreground": "#4fc3f7"},
            "output": {"foreground": "#81c784"},
            "error": {"foreground": "#ef5350"},
            "warning": {"foreground": "#ffb74d"},
            "info": {"foreground": "#90a4ae"},
            "success": {"foreground": "#66bb6a"},
            "result": {"foreground": "#26c6da", "font": ("Consolas", 10, "bold")}
        }
        
        for tag, config in tags.items():
            self.output_text.tag_configure(tag, **config)
    
    def setup_bindings(self):
        """Setup keyboard shortcuts"""
        self.input_text.bind('<Control-Return>', lambda e: self.execute_command())
        self.input_text.bind('<Control-l>', lambda e: self.clear_input())
        self.root.bind('<F1>', lambda e: self.show_help())
        self.root.bind('<F5>', lambda e: self.execute_command())
    
    def add_welcome_message(self):
        """Add welcome message"""
        self.add_output("🧮 OGULATOR Advanced Scientific Calculator", "info")
        self.add_output("Modern Python GUI with C++ Engine Backend", "info")
        self.add_output("─" * 50, "info")
        
        if self.engine_available:
            self.add_output("✅ C++ calculation engine loaded successfully", "success")
        else:
            self.add_output("🟡 C++ engine not found - using Python fallback", "warning")
            self.add_output("Click 'Build Engine' to compile the C++ backend", "info")
        
        # Package status
        available_packages = [name for name, available in self.available_packages.items() if available]
        if available_packages:
            self.add_output(f"✅ Available packages: {', '.join(available_packages)}", "success")
        
        missing_packages = [name for name, available in self.available_packages.items() if not available]
        if missing_packages:
            self.add_output(f"📦 Missing packages: {', '.join(missing_packages)}", "warning")
            self.add_output("Click 'Install Packages' to install missing scientific libraries", "info")
        
        self.add_output("Type 'help' for commands or press F1 for help dialog", "info")
        self.add_output("─" * 50, "info")
    
    def add_output(self, text, tag="output"):
        """Add text to output area"""
        self.output_text.config(state=tk.NORMAL)
        
        import datetime
        timestamp = datetime.datetime.now().strftime("%H:%M:%S")
        self.output_text.insert(tk.END, f"[{timestamp}] {text}\n", tag)
        
        self.output_text.config(state=tk.DISABLED)
        self.output_text.see(tk.END)
        
        # Store in history
        self.history.append({"text": text, "tag": tag, "timestamp": timestamp})
    
    def execute_command(self):
        """Execute command from input area"""
        command = self.input_text.get(1.0, tk.END).strip()
        if not command:
            return
        
        # Show command in output
        self.add_output(f"► {command}", "input")
        
        # Show progress
        self.set_status("Executing...")
        self.progress_bar.start()
        
        # Execute in background thread
        threading.Thread(target=self.execute_command_thread, args=(command,), daemon=True).start()
    
    def execute_command_thread(self, command):
        """Execute command in background thread"""
        try:
            if self.python_repl_mode:
                result = self.execute_python_command(command)
            else:
                result = self.execute_math_command(command)
            
            # Update UI in main thread
            self.root.after(0, self.handle_command_result, result, command)
            
        except Exception as e:
            error_result = {
                'success': False,
                'result': None,
                'error': str(e)
            }
            self.root.after(0, self.handle_command_result, error_result, command)
    
    def execute_python_command(self, command):
        """Execute Python command"""
        if command.lower() in ['exit()', 'quit()', 'exit', 'quit']:
            self.python_repl_mode = False
            return {
                'success': True,
                'result': '=== Exited Python REPL ===',
                'mode_change': True
            }
        
        try:
            # Basic Python execution with available packages
            exec_globals = {'__builtins__': __builtins__}
            
            # Add available packages
            if self.available_packages.get('numpy'):
                import numpy as np
                exec_globals['np'] = np
                exec_globals['numpy'] = np
            
            if self.available_packages.get('scipy'):
                import scipy as sp
                exec_globals['sp'] = sp
                exec_globals['scipy'] = sp
            
            # Add math
            import math
            exec_globals.update({
                'math': math, 'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
                'pi': math.pi, 'e': math.e, 'sqrt': math.sqrt, 'log': math.log
            })
            
            # Update persistent globals
            exec_globals.update(self.python_globals)
            
            try:
                # Try to evaluate as expression
                result = eval(command, exec_globals)
                self.python_globals.update(exec_globals)
                
                if result is not None:
                    return {'success': True, 'result': str(result), 'is_python': True}
                else:
                    return {'success': True, 'result': '', 'is_python': True}
            
            except SyntaxError:
                # Try to execute as statement
                exec(command, exec_globals)
                self.python_globals.update(exec_globals)
                return {'success': True, 'result': '', 'is_python': True}
        
        except Exception as e:
            return {'success': False, 'result': None, 'error': str(e), 'is_python': True}
    
    def execute_math_command(self, command):
        """Execute mathematical command with smart fallback"""
        # Handle special commands
        if command.lower() == 'help':
            self.root.after(0, self.show_help)
            return {'success': True, 'result': 'Help dialog opened', 'special': True}
        
        if command.lower().startswith('mode '):
            mode = command.split(' ', 1)[1]
            return self.change_mode_command(mode)
        
        if command.lower() in ['python', 'py', 'repl']:
            self.python_repl_mode = True
            return {
                'success': True,
                'result': '=== Python Interactive REPL Mode ===\nType exit() to return to calculator mode',
                'mode_change': True
            }
        
        # Smart fallback: Try Python first for simple arithmetic (faster)
        if self.is_simple_arithmetic(command):
            fallback_result = self.python_math_fallback(command)
            if fallback_result['success']:
                fallback_result['fast_eval'] = True
                return fallback_result
        
        # Try C++ engine for complex operations
        if self.engine_available and self.cpp_engine:
            cpp_result = self.cpp_engine.execute_command(command)
            if cpp_result['success']:
                return cpp_result
            else:
                # If C++ fails, try Python fallback
                fallback_result = self.python_math_fallback(command)
                if fallback_result['success']:
                    fallback_result['cpp_failed'] = True
                    return fallback_result
                else:
                    return cpp_result  # Return original C++ error
        
        # Pure Python fallback
        return self.python_math_fallback(command)
    
    def is_simple_arithmetic(self, command):
        """Check if command is simple arithmetic that Python can handle quickly"""
        # Simple patterns that are faster in Python
        simple_patterns = [
            # Basic arithmetic
            r'^\s*\d+\s*[+\-*/]\s*\d+\s*$',
            # Parentheses with basic operations
            r'^\s*\(\s*\d+\s*[+\-*/]\s*\d+\s*\)\s*[+\-*/]\s*\d+\s*$',
            # Power operations
            r'^\s*\d+\s*\*\*\s*\d+\s*$',
            # Multiple basic operations
            r'^\s*\d+\s*[+\-*/]\s*\d+\s*[+\-*/]\s*\d+\s*$'
        ]
        
        import re
        return any(re.match(pattern, command) for pattern in simple_patterns)
    
    def python_math_fallback(self, command):
        """Python fallback for mathematical evaluation"""
        try:
            import math
            
            # Safe mathematical evaluation
            safe_dict = {
                '__builtins__': {},
                'abs': abs, 'round': round, 'pow': pow, 'min': min, 'max': max,
                'sin': math.sin, 'cos': math.cos, 'tan': math.tan,
                'asin': math.asin, 'acos': math.acos, 'atan': math.atan,
                'log': math.log, 'log10': math.log10, 'exp': math.exp,
                'sqrt': math.sqrt, 'pi': math.pi, 'e': math.e,
                'degrees': math.degrees, 'radians': math.radians,
                'factorial': math.factorial, 'ceil': math.ceil, 'floor': math.floor
            }
            
            # Add numpy if available
            if self.available_packages.get('numpy'):
                import numpy as np
                safe_dict.update({
                    'np': np, 'array': np.array, 'sum': np.sum,
                    'mean': np.mean, 'std': np.std
                })
            
            result = eval(command, safe_dict)
            return {
                'success': True,
                'result': str(result),
                'fallback': True
            }
            
        except Exception as e:
            return {
                'success': False,
                'result': None,
                'error': str(e)
            }
    
    def handle_command_result(self, result, command):
        """Handle command result in main thread"""
        self.progress_bar.stop()
        
        if result.get('mode_change'):
            if self.python_repl_mode:
                self.input_mode_label.config(text="🐍 Python REPL")
            else:
                self.input_mode_label.config(text="Normal Mode")
        
        if result['success']:
            if result.get('result'):
                # Show result with performance metrics - SENNA STYLE! 🏎️
                result_text = result['result']
                
                if result.get('senna_speed'):
                    # MONACO GP SPEED! 🏎️
                    exec_time = result.get('execution_time', 0)
                    self.add_output(f"🏎️ {result_text} (SENNA SPEED: {exec_time}ms!)", "result")
                elif result.get('f1_speed'):
                    # F1 Championship speed! 🏁
                    exec_time = result.get('execution_time', 0)
                    self.add_output(f"🏁 {result_text} (F1 SPEED: {exec_time}ms)", "result")
                elif result.get('execution_time'):
                    exec_time = result.get('execution_time', 0)
                    self.add_output(f"🚀 {result_text} (C++ engine: {exec_time}ms)", "result")
                elif result.get('fast_eval'):
                    self.add_output(f"⚡ {result_text} (Fast Python)", "result")
                elif result.get('fallback'):
                    self.add_output(f"🐍 {result_text} (Python fallback)", "result")
                elif result.get('cpp_failed'):
                    self.add_output(f"🐍 {result_text} (C++ timeout → Python)", "result")
                elif result.get('is_python'):
                    self.add_output(f">>> {result_text}", "result")
                else:
                    self.add_output(f"🚀 {result_text} (C++ engine)", "result")
            
        else:
            error_msg = result.get('error', 'Unknown error')
            if result.get('fallback_needed'):
                # Try Python fallback automatically for this specific command
                try:
                    fallback_result = self.python_math_fallback(command)
                    if fallback_result['success']:
                        result_text = fallback_result['result']
                        self.add_output(f"🐍 {result_text} (Auto-fallback)", "result")
                        self.set_status("Ready")
                        if not self.python_repl_mode and not result.get('special'):
                            self.clear_input()
                        return  # Don't show error, fallback worked
                except:
                    pass
            
            self.add_output(f"❌ Error: {error_msg}", "error")
        
        self.set_status("Ready")
        
        # Clear input if not in Python REPL mode
        if not self.python_repl_mode and not result.get('special'):
            self.clear_input()
    
    def change_mode_command(self, mode):
        """Handle mode change via command"""
        mode_map = {
            'algebraic': 'ALGEBRAIC',
            'linear': 'LINEAR SYSTEM', 
            'stats': 'STATISTICS',
            'symbolic': 'SYMBOLIC',
            'plot': 'PLOTTING',
            'units': 'UNITS',
            'python': 'PYTHON',
            'numpy': 'NUMPY',
            'scipy': 'SCIPY',
            'matplotlib': 'MATPLOTLIB',
            'pandas': 'PANDAS',
            'sympy': 'SYMPY'
        }
        
        if mode in mode_map:
            self.current_mode = mode_map[mode]
            self.mode_label.config(text=f"Mode: {self.current_mode}")
            return {
                'success': True,
                'result': f'✓ Switched to {self.current_mode} mode'
            }
        else:
            return {
                'success': False,
                'result': None,
                'error': f'Unknown mode: {mode}'
            }
    
    def change_mode(self, mode):
        """Change mode via button"""
        self.execute_command_thread(f"mode {mode}")
    
    def clear_input(self):
        """Clear input area"""
        self.input_text.delete(1.0, tk.END)
        self.input_text.focus()
    
    def clear_history(self):
        """Clear output history"""
        if messagebox.askyesno("Clear History", "Clear all output history?"):
            self.output_text.config(state=tk.NORMAL)
            self.output_text.delete(1.0, tk.END)
            self.output_text.config(state=tk.DISABLED)
            self.history.clear()
            self.add_welcome_message()
    
    def save_results(self):
        """Save results to file"""
        from tkinter import filedialog
        filename = filedialog.asksaveasfilename(
            title="Save Results",
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        if filename:
            try:
                with open(filename, 'w', encoding='utf-8') as f:
                    for entry in self.history:
                        f.write(f"[{entry['timestamp']}] {entry['text']}\n")
                self.add_output(f"💾 Results saved to {filename}", "success")
            except Exception as e:
                self.add_output(f"❌ Save failed: {str(e)}", "error")
    
    def try_build_engine(self):
        """Try to build the C++ engine"""
        self.set_status("Building C++ engine...")
        self.progress_bar.start()
        
        def build_thread():
            try:
                current_dir = Path(__file__).parent
                
                # Try different build approaches
                try:
                    # Try fast build script first
                    if (current_dir / "fast_build.ps1").exists():
                        result = subprocess.run(
                            ["powershell", "-ExecutionPolicy", "Bypass", "./fast_build.ps1"],
                            cwd=current_dir, check=True, timeout=300, 
                            capture_output=True, text=True
                        )
                    else:
                        # Standard CMake build
                        subprocess.run(["cmake", "-B", "build", "-S", "."], 
                                     cwd=current_dir, check=True, timeout=60)
                        subprocess.run(["cmake", "--build", "build", "--parallel"], 
                                     cwd=current_dir, check=True, timeout=300)
                    
                    # Check if engine is now available
                    if self.init_cpp_engine():
                        self.engine_available = True
                        self.root.after(0, lambda: self.add_output("✅ C++ engine built and loaded successfully!", "success"))
                        self.root.after(0, lambda: self.engine_label.config(
                            text="🟢 C++ Engine Ready",
                            foreground=self.colors['accent_green']
                        ))
                    else:
                        self.root.after(0, lambda: self.add_output("❌ Engine built but executable not found", "error"))
                        
                except subprocess.CalledProcessError as e:
                    self.root.after(0, lambda: self.add_output(f"❌ Build failed: {e}", "error"))
                except subprocess.TimeoutExpired:
                    self.root.after(0, lambda: self.add_output("❌ Build timed out", "error"))
                
            except Exception as e:
                self.root.after(0, lambda: self.add_output(f"❌ Build error: {str(e)}", "error"))
            
            self.root.after(0, lambda: self.progress_bar.stop())
            self.root.after(0, lambda: self.set_status("Ready"))
        
        threading.Thread(target=build_thread, daemon=True).start()
    
    def install_packages_dialog(self):
        """Show package installation dialog"""
        install_window = tk.Toplevel(self.root)
        install_window.title("Install Python Packages")
        install_window.geometry("500x400")
        install_window.resizable(False, False)
        
        ttk.Label(install_window, text="Scientific Computing Packages", 
                 font=('Segoe UI', 12, 'bold')).pack(pady=10)
        
        # Package list
        packages_frame = ttk.Frame(install_window)
        packages_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=10)
        
        packages = {
            'numpy': 'Fundamental package for scientific computing',
            'scipy': 'Scientific computing and advanced mathematics',
            'matplotlib': 'Plotting and data visualization',
            'pandas': 'Data manipulation and analysis',
            'sympy': 'Symbolic mathematics'
        }
        
        checkboxes = {}
        for package, description in packages.items():
            frame = ttk.Frame(packages_frame)
            frame.pack(fill=tk.X, pady=2)
            
            var = tk.BooleanVar(value=not self.available_packages.get(package, False))
            checkboxes[package] = var
            
            status = "✅ Installed" if self.available_packages.get(package, False) else "⬜ Missing"
            
            ttk.Checkbutton(frame, text=f"{package} - {description}",
                           variable=var,
                           state='disabled' if self.available_packages.get(package, False) else 'normal').pack(anchor=tk.W)
            
            ttk.Label(frame, text=status, foreground='green' if self.available_packages.get(package, False) else 'red').pack(anchor=tk.W, padx=20)
        
        # Install button
        def install_selected():
            to_install = [pkg for pkg, var in checkboxes.items() if var.get() and not self.available_packages.get(pkg, False)]
            if to_install:
                install_window.destroy()
                self.install_packages(to_install)
            else:
                messagebox.showinfo("Nothing to Install", "All selected packages are already installed!")
        
        ttk.Button(install_window, text="Install Selected Packages", 
                  command=install_selected).pack(pady=20)
    
    def install_packages(self, packages):
        """Install Python packages"""
        self.set_status("Installing packages...")
        self.progress_bar.start()
        
        def install_thread():
            try:
                for package in packages:
                    self.root.after(0, lambda p=package: self.add_output(f"Installing {p}...", "info"))
                    
                    result = subprocess.run(
                        [sys.executable, "-m", "pip", "install", package],
                        capture_output=True, text=True, timeout=300
                    )
                    
                    if result.returncode == 0:
                        self.root.after(0, lambda p=package: self.add_output(f"✅ {p} installed successfully", "success"))
                    else:
                        self.root.after(0, lambda p=package, e=result.stderr: self.add_output(f"❌ Failed to install {p}: {e}", "error"))
                
                # Update package availability
                self.available_packages = self.check_packages()
                self.root.after(0, self.update_package_status)
                
            except Exception as e:
                self.root.after(0, lambda: self.add_output(f"❌ Installation error: {str(e)}", "error"))
            
            self.root.after(0, lambda: self.progress_bar.stop())
            self.root.after(0, lambda: self.set_status("Ready"))
        
        threading.Thread(target=install_thread, daemon=True).start()
    
    def update_package_status(self):
        """Update package status in UI"""
        available_count = sum(self.available_packages.values())
        total_count = len(self.available_packages)
        self.package_label.config(text=f"📦 Packages: {available_count}/{total_count}")
    
    def show_help(self):
        """Show help dialog"""
        help_window = tk.Toplevel(self.root)
        help_window.title("OGULATOR Help")
        help_window.geometry("700x500")
        
        # Help text
        help_text = scrolledtext.ScrolledText(help_window, wrap=tk.WORD, padx=10, pady=10)
        help_text.pack(fill=tk.BOTH, expand=True)
        
        help_content = """
🧮 OGULATOR - Advanced Scientific Calculator

BASIC USAGE:
• Enter mathematical expressions in the input area
• Press Ctrl+Enter or click Execute to calculate
• Results appear in the output area with timestamps

EXAMPLES:
Basic Math:
  2 + 3 * 4
  sin(pi/2)
  sqrt(16) + 2^3
  log(e)

Advanced Functions:
  factorial(5)
  abs(-10)
  round(3.14159, 2)

Python Mode:
  python          # Enter Python REPL mode
  x = 10
  y = x * 2 + 5
  exit()          # Return to calculator mode

KEYBOARD SHORTCUTS:
• Ctrl+Enter: Execute command
• Ctrl+L: Clear input
• F1: Show this help
• F5: Execute command

MODES:
• Algebraic: Basic and advanced mathematics
• Linear System: Matrix operations and solving
• Statistics: Statistical analysis functions
• Symbolic: Symbolic computation (requires SymPy)
• Plotting: Function plotting (requires Matplotlib)
• Units: Unit conversions
• Python: Interactive Python interpreter
• NumPy: Scientific computing (requires NumPy)
• SciPy: Advanced mathematics (requires SciPy)

ENGINE INFORMATION:
The calculator uses a high-performance C++ backend when available.
If the C++ engine is not built, it falls back to Python evaluation
with slightly reduced performance but full functionality.

Build the C++ engine using the "Build Engine" button for optimal performance.

PYTHON PACKAGES:
Install scientific computing packages using the "Install Packages" button:
• NumPy: Scientific computing with arrays
• SciPy: Advanced mathematical functions  
• Matplotlib: Plotting and visualization
• Pandas: Data analysis and manipulation
• SymPy: Symbolic mathematics
        """
        
        help_text.insert(tk.END, help_content)
        help_text.config(state=tk.DISABLED)
        
        # Close button
        ttk.Button(help_window, text="Close", command=help_window.destroy).pack(pady=10)
    
    def set_status(self, message):
        """Set status bar message"""
        self.status_label.config(text=message)

def main():
    """Main application entry point"""
    print("🧮 OGULATOR - Advanced Scientific Calculator")
    print("Starting Python GUI with C++ Engine Backend...")
    
    # Create main window
    root = tk.Tk()
    
    # Try to set icon (optional)
    try:
        if sys.platform.startswith('win'):
            root.iconbitmap(default='calculator.ico')
    except:
        pass
    
    # Create application
    app = OGULATORCalculatorGUI(root)
    
    # Center window on screen
    root.update_idletasks()
    width = 1200
    height = 800
    x = (root.winfo_screenwidth() // 2) - (width // 2)
    y = (root.winfo_screenheight() // 2) - (height // 2)
    root.geometry(f'{width}x{height}+{x}+{y}')
    
    print("✅ GUI initialized successfully")
    print("🚀 Starting main event loop...")
    
    # Run the application
    root.mainloop()

if __name__ == "__main__":
    main()