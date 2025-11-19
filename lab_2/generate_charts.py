#!/usr/bin/env python3
import matplotlib.pyplot as plt

data = {
    '1000x1000': {
        'threads': [1, 2, 12, 8, 16],
        'time': [0.454, 0.252, 0.264, 0.260, 0.263],
        'speedup': [1.00, 1.80, 1.72, 1.74, 1.72],
        'efficiency': [100.00, 90.00, 14.00, 21.00, 10.00]
    },
    '2000x2000': {
        'threads': [1, 2, 12, 8, 16],
        'time': [1.800, 0.999, 0.606, 0.603, 0.605],
        'speedup': [1.00, 1.80, 2.96, 2.98, 2.97],
        'efficiency': [100.00, 90.00, 24.00, 37.00, 18.00]
    },
    '3000x3000': {
        'threads': [1, 2, 12, 8, 16],
        'time': [4.040, 2.236, 1.034, 1.236, 1.034],
        'speedup': [1.00, 1.80, 3.90, 3.26, 3.90],
        'efficiency': [100.00, 90.00, 32.00, 40.00, 24.00]
    }
}

fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))

colors = ['blue', 'green', 'red']
for idx, (matrix_size, values) in enumerate(data.items()):
    ax1.plot(values['threads'], values['time'], marker='o', label=matrix_size, 
             color=colors[idx], linewidth=2, markersize=8)
ax1.set_xlabel('Количество потоков')
ax1.set_ylabel('Время выполнения (с)')
ax1.set_title('Зависимость времени выполнения от количества потоков')
ax1.legend()
ax1.grid(True, alpha=0.3)
ax1.set_xticks([1, 2, 8, 12, 16])

for idx, (matrix_size, values) in enumerate(data.items()):
    ax2.plot(values['threads'], values['speedup'], marker='s', label=matrix_size,
             color=colors[idx], linewidth=2, markersize=8)

ideal_threads = [1, 2, 8, 12, 16]
ideal_speedup = [min(t, 16) for t in ideal_threads]
ax2.plot(ideal_threads, ideal_speedup, 'k--', label='Идеальное ускорение', alpha=0.7, linewidth=2)
ax2.set_xlabel('Количество потоков')
ax2.set_ylabel('Коэффициент ускорения')
ax2.set_title('Зависимость ускорения от количества потоков')
ax2.legend()
ax2.grid(True, alpha=0.3)
ax2.set_xticks([1, 2, 8, 12, 16])

for idx, (matrix_size, values) in enumerate(data.items()):
    ax3.plot(values['threads'], values['efficiency'], marker='^', label=matrix_size,
             color=colors[idx], linewidth=2, markersize=8)
ax3.set_xlabel('Количество потоков')
ax3.set_ylabel('Эффективность (%)')
ax3.set_title('Зависимость эффективности от количества потоков')
ax3.legend()
ax3.grid(True, alpha=0.3)
ax3.set_xticks([1, 2, 8, 12, 16])

cores = 12
efficiency_at_cores = {}
for matrix_size, values in data.items():
    idx = values['threads'].index(cores)
    efficiency_at_cores[matrix_size] = values['efficiency'][idx]

matrix_sizes = list(efficiency_at_cores.keys())
efficiencies = list(efficiency_at_cores.values())

bars = ax4.bar(matrix_sizes, efficiencies, color=['skyblue', 'lightgreen', 'lightcoral'])
ax4.set_xlabel('Размер матрицы')
ax4.set_ylabel('Эффективность (%)')
ax4.set_title(f'Эффективность при использовании {cores} потоков\n(равно количеству ядер)')
ax4.grid(True, alpha=0.3)

for bar, efficiency in zip(bars, efficiencies):
    ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1,
             f'{efficiency}%', ha='center', va='bottom', fontweight='bold')

plt.tight_layout()
plt.savefig('performance_analysis.png', dpi=300, bbox_inches='tight')
plt.show()

print("=== АНАЛИЗ РЕЗУЛЬТАТОВ ===")
print("\n1. ОБЩАЯ ПРОИЗВОДИТЕЛЬНОСТЬ:")
print(f"   - Максимальное ускорение: {max(data['3000x3000']['speedup']):.2f}x (3000x3000, 12 потоков)")
print(f"   - Лучшая эффективность: {max(data['1000x1000']['efficiency']):.1f}% (1000x1000, 2 потока)")

print("\n2. ОПТИМАЛЬНОЕ КОЛИЧЕСТВО ПОТОКОВ:")
for size in data.keys():
    best_idx = data[size]['speedup'].index(max(data[size]['speedup']))
    best_threads = data[size]['threads'][best_idx]
    best_speedup = data[size]['speedup'][best_idx]
    best_eff = data[size]['efficiency'][best_idx]
    print(f"   - {size}: {best_threads} потоков (ускорение: {best_speedup}x, эффективность: {best_eff}%)")

print("\n3. АНАЛИЗ ПРИ K = 12 (КОЛИЧЕСТВО ЯДЕР):")
for size, eff in efficiency_at_cores.items():
    print(f"   - {size}: эффективность {eff}%")

print("\n4. ВЫВОДЫ:")
print("   - 2 потока дают наивысшую эффективность (80-90%)")
print("   - 12 потоков (число ядер) дают максимальное ускорение для больших матриц")
print("   - Эффективность падает при большом количестве потоков из-за накладных расходов")
print("   - Закон Амдала: последовательная часть ограничивает максимальное ускорение")
