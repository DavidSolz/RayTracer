import matplotlib.pyplot as plt
import numpy as np

file_path = 'Performance_log.csv'
data = []

with open(file_path, 'r') as file:

    next(file)
    next(file)

    for line in file:
        fps, frametime = map(float, line.strip().split(';'))
        data.append((fps, frametime))

data = data[1::]

fps, frametime = zip(*data)

mean_fps = np.mean(fps)
std_fps = np.std(fps)
var_fps = np.var(fps)

mean_frametime = np.mean(frametime)
std_frametime = np.std(frametime)
var_frametime = np.var(frametime)


plt.subplot(3, 1, 1)
plt.plot(fps, linestyle='-', color='b')
plt.axhline(mean_fps, linestyle='--', color='k', label='Mean FPS')
plt.fill_between(range(len(fps)), mean_fps - std_fps, mean_fps + std_fps, color='b', alpha=0.2, label='FPS ± Std Dev')
plt.xlabel('Sample')
plt.ylabel('FPS')
plt.legend()
plt.grid(True)

plt.subplot(3, 1, 2)
plt.plot(frametime, linestyle='-', color='r')
plt.axhline(mean_frametime, linestyle='--', color='k', label='Mean Frametime')
plt.fill_between(range(len(frametime)), mean_frametime - std_frametime, mean_frametime + std_frametime, color='r', alpha=0.2, label='Frametime ± Std Dev')
plt.xlabel('Sample')
plt.ylabel('Frametime (ms)')
plt.legend()
plt.grid(True)

plt.subplot(3, 2, 5)
plt.hist(fps, bins=20, color='b', alpha=0.7)
plt.axvline(mean_fps, color='k', linestyle='--', linewidth=2, label='Mean FPS')
plt.xlabel('FPS')
plt.ylabel('Frequency')
plt.legend()
plt.grid(True)

plt.subplot(3, 2, 6)
plt.hist(frametime, bins=20, color='r', alpha=0.7)
plt.axvline(mean_frametime, color='k', linestyle='--', linewidth=2, label='Mean Frametime')
plt.xlabel('Frametime (ms)')
plt.ylabel('Frequency')
plt.legend()
plt.grid(True)

plt.tight_layout()
# plt.show()

plt.savefig('performance_plots.png')

plt.close()
