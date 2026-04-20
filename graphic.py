import matplotlib.pyplot as plt
import numpy as np

labels = ['Base_O0', 'Base_O3', 'crc32', 'strlen', 'strcmp', 'pgo']
full_time = [21.707, 17.709, 15.54, 15.403, 15.37, 15.72]

x = np.arange(len(labels))

fig, ax = plt.subplots(figsize=(12, 7), dpi=300)

rects1 = ax.bar(x, full_time, width, label='Full time', color='#DA70D6')

ax.set_ylabel('Ticks (milliards)')
ax.set_title('Hash-tables Optimizations')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

ax.bar_label(rects1, padding=3)

fig.tight_layout()
plt.savefig("graphic.png")