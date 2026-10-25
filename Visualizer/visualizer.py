import re
import subprocess
from matplotlib.figure import Figure
import numpy as np
from io import BytesIO

#Upload and save file to username in Uploads
filename = "hans"

# Strip out #defines and #includes
with open("Visualizer/vis/Uploads/" + filename + ".ino") as file:
  code_input = file.read()

cleaned = re.sub(r'^\s*#\w+.*$', '', code_input, flags=re.MULTILINE)

# Remove extern C
cleaned = re.sub(r'^\s*extern+.*$', '', cleaned, flags=re.MULTILINE)

# Remove Serial calls
cleaned = re.sub(r'^\s*Serial+.*$', '', cleaned, flags=re.MULTILINE)

#write the file
with open("Visualizer/vis/Uploads/" + filename + ".cpp", "w+") as f:
    f.write('#include "arduino_stubs.h"\n')
    f.write('#include <cmath>\n')
    f.write(cleaned)

# TODO: Add timeout!
subprocess.call("Visualizer/bw_cmd.sh")

#plot the data

# Assumes columns: time_us,grad_y,grad_z,rf_amp
data = np.loadtxt("Visualizer/vis/Data/" + filename + ".csv", delimiter=",", skiprows=1)

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
fig.savefig('himage.png')

# Delete files afterward
os.remove("Visualizer/vis/Data/" + filename + ".csv")
os.remove("Visualizer/vis/Uploads/" + filename + ".cpp")
# os.remove("Visualizer/vis/Uploads/" + filename + ".ino")