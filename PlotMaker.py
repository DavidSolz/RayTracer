import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import gaussian_kde
from scipy.ndimage import gaussian_filter1d

file_path = 'Performance_log.csv'
data = []

with open(file_path, 'r') as file:
    next(file)
    next(file)

    for line in file:
        fps, frametime, _ = map(float, line.strip().split(';'))
        data.append((fps, frametime))

fps, frametime = zip(*data)

indices_to_plot = list(range(0, len(fps)))

selected_fps = [fps[i] for i in indices_to_plot]
selected_frametime = [frametime[i] for i in indices_to_plot]

mean_fps = np.mean(selected_fps)
std_fps = np.std(selected_fps)
mean_frametime = np.mean(selected_frametime)
std_frametime = np.std(selected_frametime)

plt.figure(figsize=(10, 8))

# FPS plot
plt.subplot(2, 2, 1)
plt.plot(indices_to_plot, fps, linestyle='-', color='b')
plt.axhline(mean_fps, linestyle='--', color='k', label='Mean FPS')
plt.fill_between(indices_to_plot, mean_fps - std_fps, mean_fps + std_fps, color='b', alpha=0.2, label='FPS ± Std Dev')
plt.xlabel('Sample Index')
plt.ylabel('FPS')
plt.legend()

# Frametime plot
plt.subplot(2, 2, 2)
plt.plot(indices_to_plot, frametime, linestyle='-', color='r')
plt.axhline(mean_frametime, linestyle='--', color='k', label='Mean Frametime')
plt.fill_between(indices_to_plot, mean_frametime - std_frametime, mean_frametime + std_frametime, color='r', alpha=0.2, label='Frametime ± Std Dev')
plt.xlabel('Sample Index')
plt.ylabel('Frametime (ms)')
plt.legend()

# FPS Histogram
plt.subplot(2, 2, 3)
plt.hist(selected_fps, bins=20, color='b', alpha=0.7, density=True)

# kde_fps = gaussian_kde(selected_fps)
# x_vals = np.linspace(min(selected_fps), max(selected_fps), 100)
# plt.plot(x_vals, kde_fps(x_vals), color='k', linestyle='-', linewidth=1)

plt.axvline(mean_fps, color='r', linestyle='--', linewidth=1, label='Mean FPS')
plt.xlabel('FPS')
plt.ylabel('Density')
plt.legend()

# Frametime Histogram
plt.subplot(2, 2, 4)
plt.hist(selected_frametime, bins=20, color='r', alpha=0.7, density=True)

# kde_frametime = gaussian_kde(selected_frametime)
# x_vals = np.linspace(min(selected_frametime), max(selected_frametime), 100)
# plt.plot(x_vals, kde_frametime(x_vals), color='k', linestyle='-', linewidth=1)

plt.axvline(mean_frametime, color='b', linestyle='--', linewidth=1, label='Mean Frametime')
plt.xlabel('Frametime (ms)')
plt.ylabel('Density')
plt.legend()

plt.tight_layout()
plt.show()
