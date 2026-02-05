#include "Sys_load.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ==========================================
// РЕАЛИЗАЦИЯ ДЛЯ LINUX
// ==========================================
#ifndef _WIN32

#include <unistd.h>

// Вспомогательная функция чтения /proc/stat
static int read_linux_stats(CoreState* stats, int max_len) {
    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) return 0;

    char buf[1024];
    int count = 0;
    while (fgets(buf, sizeof(buf), fp) && count < max_len) {
        if (strncmp(buf, "cpu", 3) == 0 && buf[3] >= '0' && buf[3] <= '9') {
            sscanf(buf, "%*s %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats[count].user, &stats[count].nice, &stats[count].system,
                   &stats[count].idle, &stats[count].iowait, &stats[count].irq,
                   &stats[count].softirq, &stats[count].steal);
            count++;
        }
    }
    fclose(fp);
    return count;
}

void cpu_monitor_init(CpuMonitor* monitor) {
    memset(monitor, 0, sizeof(CpuMonitor));
    monitor->num_cores = read_linux_stats(monitor->prev_stats, MAX_CORES);
}

int cpu_monitor_update(CpuMonitor* monitor, float* usage_per_core) {
    CoreState cur_stats[MAX_CORES];
    int count = read_linux_stats(cur_stats, MAX_CORES);

    for (int i = 0; i < count; i++) {
        unsigned long long prev_idle = monitor->prev_stats[i].idle + monitor->prev_stats[i].iowait;
        unsigned long long cur_idle = cur_stats[i].idle + cur_stats[i].iowait;

        unsigned long long prev_total = prev_idle + monitor->prev_stats[i].user + monitor->prev_stats[i].nice + 
                                        monitor->prev_stats[i].system + monitor->prev_stats[i].irq + 
                                        monitor->prev_stats[i].softirq + monitor->prev_stats[i].steal;
                                        
        unsigned long long cur_total = cur_idle + cur_stats[i].user + cur_stats[i].nice + 
                                       cur_stats[i].system + cur_stats[i].irq + 
                                       cur_stats[i].softirq + cur_stats[i].steal;

        unsigned long long total_diff = cur_total - prev_total;
        unsigned long long idle_diff = cur_idle - prev_idle;

        if (total_diff > 0) {
            usage_per_core[i] = (float)((total_diff - idle_diff) * 100.0 / total_diff);
        } else {
            usage_per_core[i] = 0.0f;
        }

        // Обновляем "предыдущее" состояние на текущее
        monitor->prev_stats[i] = cur_stats[i];
    }
    return count;
}

// ==========================================
// РЕАЛИЗАЦИЯ ДЛЯ WINDOWS
// ==========================================
#else 

#include <winternl.h>

// Определения для NtQuerySystemInformation (обычно нужны, если нет в стандартных хедерах)
typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_STRUCT {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} SPPI;

// Указатель на функцию API
typedef NTSTATUS(WINAPI *PNT_QUERY)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

void cpu_monitor_init(CpuMonitor* monitor) {
    memset(monitor, 0, sizeof(CpuMonitor));
    
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    monitor->num_cores = sysInfo.dwNumberOfProcessors;
    if (monitor->num_cores > MAX_CORES) monitor->num_cores = MAX_CORES;

    // Делаем первый "пустой" замер, чтобы инициализировать prev_stats
    HMODULE hNtdll = GetModuleHandle("ntdll.dll");
    PNT_QUERY NtQuery = (PNT_QUERY)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    
    if (NtQuery) {
        NtQuery(SystemProcessorPerformanceInformation, 
                monitor->prev_stats, 
                sizeof(SPPI) * monitor->num_cores, 
                NULL);
    }
}

int cpu_monitor_update(CpuMonitor* monitor, float* usage_per_core) {
    SPPI cur_stats[MAX_CORES];
    
    HMODULE hNtdll = GetModuleHandle("ntdll.dll");
    PNT_QUERY NtQuery = (PNT_QUERY)GetProcAddress(hNtdll, "NtQuerySystemInformation");

    if (!NtQuery) return 0;

    NtQuery(SystemProcessorPerformanceInformation, cur_stats, sizeof(SPPI) * monitor->num_cores, NULL);

    for (int i = 0; i < monitor->num_cores; i++) {
        long long idle = cur_stats[i].IdleTime.QuadPart - monitor->prev_stats[i].IdleTime.QuadPart;
        long long kernel = cur_stats[i].KernelTime.QuadPart - monitor->prev_stats[i].KernelTime.QuadPart;
        long long user = cur_stats[i].UserTime.QuadPart - monitor->prev_stats[i].UserTime.QuadPart;

        // В Windows KernelTime включает IdleTime
        long long total = kernel + user;
        long long used = total - idle;

        if (total > 0) {
            usage_per_core[i] = (float)(used * 100.0 / total);
        } else {
            usage_per_core[i] = 0.0f;
        }

        // Сохраняем для следующего раза
        monitor->prev_stats[i].IdleTime = cur_stats[i].IdleTime;
        monitor->prev_stats[i].KernelTime = cur_stats[i].KernelTime;
        monitor->prev_stats[i].UserTime = cur_stats[i].UserTime;
    }

    return monitor->num_cores;
}

#endif