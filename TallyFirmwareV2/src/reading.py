import queue
import socket
import threading
import tkinter as tk
from tkinter import messagebox, scrolledtext, ttk


class UdpMonitor:
    def __init__(self, root):
        self.root = root
        self.root.title("Accelerometer UDP Monitor")
        self.socket = None
        self.running = False
        self.messages = queue.Queue()

        controls = ttk.Frame(root, padding=8)
        controls.pack(fill="x")

        ttk.Label(controls, text="Listen IP").grid(row=0, column=0, padx=4)
        self.ip_entry = ttk.Entry(controls, width=16)
        self.ip_entry.insert(0, "0.0.0.0")
        self.ip_entry.grid(row=0, column=1, padx=4)

        ttk.Label(controls, text="Port").grid(row=0, column=2, padx=4)
        self.port_entry = ttk.Entry(controls, width=8)
        self.port_entry.insert(0, "5198")
        self.port_entry.grid(row=0, column=3, padx=4)

        self.start_button = ttk.Button(controls, text="Listen", command=self.start)
        self.start_button.grid(row=0, column=4, padx=4)
        self.stop_button = ttk.Button(
            controls, text="Stop", command=self.stop, state="disabled"
        )
        self.stop_button.grid(row=0, column=5, padx=4)

        self.status = ttk.Label(root, text="Not listening")
        self.status.pack(anchor="w", padx=12)

        self.output = scrolledtext.ScrolledText(root, width=90, height=20)
        self.output.pack(fill="both", expand=True, padx=8, pady=8)

        self.root.protocol("WM_DELETE_WINDOW", self.close)
        self.root.after(100, self.show_messages)

    def start(self):
        try:
            listen_ip = self.ip_entry.get().strip()
            port = int(self.port_entry.get())
            udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            udp_socket.bind((listen_ip, port))
        except (ValueError, OSError) as error:
            messagebox.showerror("Could not listen", str(error))
            return

        self.socket = udp_socket
        self.running = True
        self.status.config(text=f"Listening on {listen_ip}:{port}")
        self.start_button.config(state="disabled")
        self.stop_button.config(state="normal")
        threading.Thread(target=self.receive, args=(udp_socket,), daemon=True).start()

    def receive(self, udp_socket):
        while self.running:
            try:
                data, address = udp_socket.recvfrom(1024)
            except OSError:
                break

            values = data.decode("utf-8", errors="replace").strip()
            parts = values.split(",")
            if len(parts) == 6:
                labels = ("VP", "VN", "D34", "D35", "D32", "D33")
                values = "  ".join(
                    f"{label}={value} g" for label, value in zip(labels, parts)
                )
            self.messages.put(f"{address[0]}:{address[1]}  {values}")

    def show_messages(self):
        while not self.messages.empty():
            self.output.insert("end", self.messages.get_nowait() + "\n")
            self.output.see("end")
        self.root.after(100, self.show_messages)

    def stop(self):
        self.running = False
        if self.socket is not None:
            self.socket.close()
            self.socket = None
        self.status.config(text="Not listening")
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")

    def close(self):
        self.stop()
        self.root.destroy()


if __name__ == "__main__":
    window = tk.Tk()
    UdpMonitor(window)
    window.mainloop()
