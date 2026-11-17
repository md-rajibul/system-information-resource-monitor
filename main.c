#include <sys/statvfs.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

void showSystemInfo()
{
    struct utsname system;

    if (uname(&system) == 0) {
        printf("\nSystem Name  : %s", system.sysname);
        printf("\nHost Name    : %s", system.nodename);
        printf("\nKernel       : %s", system.release);
        printf("\nArchitecture : %s\n", system.machine);
    }

    FILE *fp = fopen("/proc/uptime", "r");

    if (fp != NULL) {
        double uptime;

        if (fscanf(fp, "%lf", &uptime) == 1) {
            printf("System Uptime: %.0f seconds\n", uptime);
        }

        fclose(fp);
    }
}

void showMemoryInfo()
{
    FILE *fp = fopen("/proc/meminfo", "r");

    if (fp == NULL) {
        printf("Unable to read memory information.\n");
        return;
    }

    char key[100];
    unsigned long value;
    char unit[20];

    unsigned long total = 0;
    unsigned long available = 0;

    while (fscanf(fp, "%99s %lu %19s", key, &value, unit) == 3) {

        if (strcmp(key, "MemTotal:") == 0)
            total = value;

        if (strcmp(key, "MemAvailable:") == 0)
            available = value;
    }

    fclose(fp);

    if (total > 0) {
        printf("\n========== MEMORY INFORMATION ==========\n");
        printf("Total RAM     : %.2f MB\n", total / 1024.0);
        printf("Available RAM : %.2f MB\n", available / 1024.0);
        printf("Used RAM      : %.2f MB\n", (total - available) / 1024.0);
        printf("RAM Usage     : %.2f%%\n",
               ((double)(total - available) / total) * 100);
    }
}

void showCPUInfo()
{
    FILE *fp = fopen("/proc/cpuinfo", "r");

    if (fp == NULL) {
        printf("Unable to read CPU information.\n");
        return;
    }

    char line[256];

    printf("\n========== CPU INFORMATION ==========\n");

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            printf("%s", line);
            break;
        }
    }

    fclose(fp);
}
 void showProcessInfo()
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/proc");

    if (dir == NULL) {
        printf("Unable to open /proc directory.\n");
        return;
    }

    printf("\n========== RUNNING PROCESS INFORMATION ==========\n");
    printf("%-10s %-25s %-10s\n", "PID", "PROCESS NAME", "STATE");
    printf("--------------------------------------------------\n");

    int count = 0;

    while ((entry = readdir(dir)) != NULL) {

        if (!isdigit((unsigned char)entry->d_name[0])) {
            continue;
        }

        char path[4096];
        char name[256];
        char state;

        snprintf(path, sizeof(path),
                 "/proc/%s/stat", entry->d_name);

        FILE *fp = fopen(path, "r");

        if (fp == NULL) {
            continue;
        }

        int pid;

        if (fscanf(fp, "%d (%255[^)]) %c",
                   &pid, name, &state) == 3) {

            printf("%-10d %-25.25s %-10c\n",
                   pid, name, state);

            count++;
        }

        fclose(fp);

        if (count >= 20) {
            break;
        }
    }

    closedir(dir);

    printf("--------------------------------------------------\n");
    printf("Total Processes Displayed: %d\n", count);
}
void showCPUUsage()
{
    FILE *fp;
    char line[256];

    unsigned long long user1, nice1, system1, idle1;
    unsigned long long iowait1, irq1, softirq1, steal1;

    unsigned long long user2, nice2, system2, idle2;
    unsigned long long iowait2, irq2, softirq2, steal2;

    fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        printf("Unable to read CPU information.\n");
        return;
    }

    if (fgets(line, sizeof(line), fp) == NULL ||
        sscanf(line,
               "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user1, &nice1, &system1, &idle1,
               &iowait1, &irq1, &softirq1, &steal1) != 8) {
        printf("Unable to parse CPU information.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    sleep(1);

    fp = fopen("/proc/stat", "r");

    if (fp == NULL) {
        printf("Unable to read CPU information.\n");
        return;
    }

    if (fgets(line, sizeof(line), fp) == NULL ||
        sscanf(line,
               "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user2, &nice2, &system2, &idle2,
               &iowait2, &irq2, &softirq2, &steal2) != 8) {
        printf("Unable to parse CPU information.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    unsigned long long idleTime1 = idle1 + iowait1;
    unsigned long long idleTime2 = idle2 + iowait2;

    unsigned long long total1 =
        user1 + nice1 + system1 + idle1 +
        iowait1 + irq1 + softirq1 + steal1;

    unsigned long long total2 =
        user2 + nice2 + system2 + idle2 +
        iowait2 + irq2 + softirq2 + steal2;

    unsigned long long totalDiff = total2 - total1;
    unsigned long long idleDiff = idleTime2 - idleTime1;

    if (totalDiff == 0 || idleDiff > totalDiff) {
        printf("Unable to calculate CPU usage.\n");
        return;
    }

    double idlePercentage =
        (double)idleDiff / totalDiff * 100.0;

    double cpuUsage = 100.0 - idlePercentage;

    printf("\n========== CPU USAGE MONITORING ==========\n");
    printf("CPU Usage : %.2f%%\n", cpuUsage);
    printf("CPU Idle  : %.2f%%\n", idlePercentage);
}
void showStorageInfo()
{
    struct statvfs storage;

    if (statvfs("/", &storage) != 0) {
        perror("Unable to read storage information");
        return;
    }

    unsigned long long blockSize = storage.f_frsize;

    unsigned long long totalBytes =
        (unsigned long long)storage.f_blocks * blockSize;

    unsigned long long freeBytes =
        (unsigned long long)storage.f_bfree * blockSize;

    unsigned long long availableBytes =
        (unsigned long long)storage.f_bavail * blockSize;

    unsigned long long usedBytes = totalBytes - freeBytes;

    double totalGB = totalBytes / (1024.0 * 1024.0 * 1024.0);
    double usedGB = usedBytes / (1024.0 * 1024.0 * 1024.0);
    double availableGB = availableBytes / (1024.0 * 1024.0 * 1024.0);

    double usagePercentage = 0.0;

    if (totalBytes > 0) {
        usagePercentage =
            ((double)usedBytes / totalBytes) * 100.0;
    }

    printf("\n========== STORAGE INFORMATION ==========\n");
    printf("Filesystem       : /\n");
    printf("Total Storage    : %.2f GB\n", totalGB);
    printf("Used Storage     : %.2f GB\n", usedGB);
    printf("Available Storage: %.2f GB\n", availableGB);
    printf("Storage Usage    : %.2f%%\n", usagePercentage);
}
int main(void)
{
    int choice;

    while (1)
    {
        printf("\n========================================\n");
        printf(" SYSTEM INFORMATION & RESOURCE MONITOR\n");
        printf("========================================\n");

        printf("\n             MAIN MENU\n\n");

        printf("1. View System Information\n");
        printf("2. View CPU Information\n");
        printf("3. View Memory Information\n");
        printf("4. View Running Processes\n");
        printf("5. View CPU Usage\n");
        printf("6. View Storage Information\n");
        printf("7. View All Information\n");
        printf("0. Exit\n");

        printf("\n========================================\n");
        printf("Enter Your Choice: ");

        if (scanf("%d", &choice) != 1)
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }

            printf("\nInvalid input! Enter a number.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
                showSystemInfo();
                break;

            case 2:
                showCPUInfo();
                break;

            case 3:
                showMemoryInfo();
                break;

            case 4:
                showProcessInfo();
                break;

            case 5:
                showCPUUsage();
                break;

            case 6:
                showStorageInfo();
                break;

            case 7:
                showSystemInfo();
                showCPUInfo();
                showMemoryInfo();
                showProcessInfo();
                showCPUUsage();
                showStorageInfo();
                break;

            case 0:
                printf("\nThank you for using System Monitor!\n");
                return 0;

            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    }
}



