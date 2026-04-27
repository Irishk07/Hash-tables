import matplotlib.pyplot as plt
import numpy as np

labels = ['Base_O0', 'Base_O3', 'crc32', 'strlen', 'strcmp', 'pgo']
full_time = [2.306, 1.76, 1.56, 1.55, 1.54, 1.57]

x = np.arange(len(labels))
width = 0.6

fig, ax = plt.subplots(figsize=(12, 7), dpi=300)

rects1 = ax.bar(x, full_time, width, label='Full time', color='#DA70D6')

ax.set_ylabel('Ticks (*10^10)')
ax.set_title('Hash-tables Optimizations')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

ax.bar_label(rects1, padding=3, fmt='%.3f')

fig.tight_layout()
plt.savefig("graphic.png")