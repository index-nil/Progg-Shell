#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H

// Максимальное количество поддерживаемых ядер (можно увеличить)
#define MAX_CORES 128

// Платформо-зависимое хранение состояния
#ifdef _WIN32
    #include <windows.h>
    typedef struct {
        LARGE_INTEGER IdleTime;
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
    } CoreState;
#else
    typedef struct {
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    } CoreState;
#endif

// Основная структура монитора
typedef struct {
    int num_cores;
    CoreState prev_stats[MAX_CORES]; // Хранит предыдущий снимок
} CpuMonitor;

// === API Функции ===

// Инициализация (определяет кол-во ядер и делает первый замер)
void cpu_monitor_init(CpuMonitor* monitor);

// Получение нагрузки. 
// Заполняет массив usage_per_core процентами (0.0 - 100.0)
// Возвращает количество ядер.
int cpu_monitor_update(CpuMonitor* monitor, float* usage_per_core);

#endif // CPU_MONITOR_H

