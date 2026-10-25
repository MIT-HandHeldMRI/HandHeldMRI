import os
import uuid
import subprocess
from flask import Flask, render_template, request
from flask_httpauth import HTTPBasicAuth
import re
from matplotlib.figure import Figure
import numpy as np
from io import BytesIO
import base64

# --- Configuration ---
UPLOAD_FOLDER = '/home/mrilab/64800_MRI/Visualizer/vis/Uploads/'
OUTPUT_FOLDER = '/home/mrilab/64800_MRI/Visualizer/vis/static/images/'
DATA_FOLDER = '/home/mrilab/64800_MRI/Visualizer/vis/Data/'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(OUTPUT_FOLDER, exist_ok=True)

PASSWORD = "HandHeldMRI"  # 🔐 Your hardcoded password

app = Flask(__name__)
auth = HTTPBasicAuth()

# --- Basic Auth setup ---
@auth.verify_password
def verify_password(username, password):
    # Ignore username, only require correct password
    if password == PASSWORD:
        return True
    return False


# --- Routes ---
@app.route('/', methods=['GET', 'POST'])
@auth.login_required
def index():
    image_data_urls = []
    error = None

    if request.method == 'POST':
        if 'file' not in request.files:
            error = "No file uploaded"
        else:
            file = request.files['file']
            iterations = int(request.form.get('iterations', 1))
            iterations = max(1, min(iterations, 80))

            unique_id = str(uuid.uuid4())
            filename = f"{unique_id}_{file.filename}"
            filepath = os.path.join(UPLOAD_FOLDER, filename)
            file.save(filepath)
            
            try:
                #Process file and run simulation
                with open(filepath) as file:
                    code_input = file.read()

                cleaned = re.sub(r'^\s*#include\s+.*$', '', code_input, flags=re.MULTILINE)

                # Remove extern C
                cleaned = re.sub(r'^\s*extern+.*$', '', cleaned, flags=re.MULTILINE)

                # Remove Serial calls
                cleaned = re.sub(r'^\s*Serial+.*$', '', cleaned, flags=re.MULTILINE)

                # Waits using ARM cycle counter
                cleaned = re.sub(r'^\s*while\s*\(\s*\(uint64_t\)\s*ARM_DWT_CYCCNT\s*<\s*([^{]+)\)\s*{\s*}', 
                r'ArduinoSim::current_time_us = (\1)/600;', cleaned, flags=re.MULTILINE)

                #write a "clean" cpp file
                with open(UPLOAD_FOLDER + unique_id + ".cpp", "w+") as f:
                    f.write('#include "arduino_stubs.h"\n')
                    f.write('#include <cmath>\n')
                    f.write(cleaned)

                subprocess.call(["/home/mrilab/64800_MRI/Visualizer/bw_cmd.sh", str(unique_id)])
                #plot the data
                image_buffers = []
                for t in range(1,iterations+1):
                    # Assumes columns: time_us,grad_y,grad_z,rf_amp
                    data = np.loadtxt(DATA_FOLDER + unique_id + f"/log_{t}.csv", delimiter=",", skiprows=1)

                    time = data[:, 0]
                    grad_y = data[:, 1]
                    grad_z = data[:, 2]
                    rf_amp = data[:, 3]

                    fig = Figure(figsize=(12, 6))
                    ax = fig.subplots()

                    # Plot gradients as lines
                    ax.plot(time, grad_y, label='Gradient Y', color='blue', drawstyle='steps-post')
                    ax.plot(time, grad_z, label='Gradient Z', color='green', drawstyle='steps-post')

                    # Plot RF pulses as rectangles
                    for i in range(1, len(time)):
                        # Detect transitions in RF amplitude
                        if rf_amp[i] != rf_amp[i-1]:
                            if rf_amp[i] > 0:
                                # rectangle from current to next time step
                                t_start = time[i]
                                t_end = time[i+1] if i+1 < len(time) else time[i]
                                ax.fill_between(
                                    [t_start, t_end],
                                    -rf_amp[i], rf_amp[i],
                                    color='red', alpha=0.3, step='post'
                                )

                    ax.set_xlabel("Time (µs)")
                    ax.set_ylabel("Amplitude")
                    ax.set_title("Simulated RF and Gradient Waveforms")
                    ax.legend()
                    ax.grid(True)
                    fig.tight_layout()
                    # Save it to a temporary buffer.
                    # buf = BytesIO()
                    buf = BytesIO()
                    # fig.savefig(f"{OUTPUT_FOLDER}scan{t}.png", format="png")
                    fig.savefig(buf, format="png")
                    image_buffers.append(buf)

                # Collect generated image paths
                for buf in image_buffers:
                    buf.seek(0)
                    img_base64 = base64.b64encode(buf.read()).decode('utf-8')
                    image_data_urls.append(f"data:image/png;base64,{img_base64}")

            except Exception as e:
                print(e)
                error = "Simulation Failed! Check your code"
                image_paths = []

            subprocess.call(["rm", "-rf",  DATA_FOLDER + unique_id])
            os.remove(UPLOAD_FOLDER + unique_id + ".cpp")
            os.remove(filepath)

    return render_template('index.html', image_paths=image_data_urls, error=error)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)

