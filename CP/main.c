#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define MAX_JOBS 50
#define MAX_DEPS 10
#define MAX_BARRIERS 10

typedef struct {
    char name[32];
    char command[256];
    char deps[MAX_DEPS][32];
    int deps_count;
    int done_deps;
    char barrier[32];
    int done;
    pthread_t thread;
    pid_t pid;
} Job;

typedef struct {
    char name[32];
    int total;
    int done;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Barrier;

Job jobs[MAX_JOBS];
Barrier barriers[MAX_BARRIERS];
int job_cnt = 0;
int barrier_cnt = 0;
int failed = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// ===================== ПРОВЕРКИ =====================

int find_job(char* name) {
    for (int i = 0; i < job_cnt; i++)
        if (strcmp(jobs[i].name, name) == 0) return i;
    return -1;
}

Barrier* find_barrier(char* name) {
    for (int i = 0; i < barrier_cnt; i++)
        if (strcmp(barriers[i].name, name) == 0) return &barriers[i];
    return NULL;
}

int check_cycles() {
    int in[MAX_JOBS] = {0};
    
    for (int i = 0; i < job_cnt; i++) {
        for (int d = 0; d < jobs[i].deps_count; d++) {
            int dep = find_job(jobs[i].deps[d]);
            if (dep == -1) {
                printf("Error: job %s depends on unknown %s\n", jobs[i].name, jobs[i].deps[d]);
                return 0;
            }
            in[i]++;
        }
    }
    
    int q[MAX_JOBS], front = 0, rear = 0;
    for (int i = 0; i < job_cnt; i++)
        if (in[i] == 0) q[rear++] = i;
    
    int processed = 0;
    while (front < rear) {
        int u = q[front++];
        processed++;
        
        for (int i = 0; i < job_cnt; i++) {
            for (int d = 0; d < jobs[i].deps_count; d++) {
                if (strcmp(jobs[i].deps[d], jobs[u].name) == 0) {
                    if (--in[i] == 0) q[rear++] = i;
                }
            }
        }
    }
    
    if (processed != job_cnt) {
        printf("Error: cycle detected\n");
        return 0;
    }
    return 1;
}

int check_connected() {
    if (job_cnt == 0) return 1;
    
    int visited[MAX_JOBS] = {0};
    int stack[MAX_JOBS], top = 0;
    stack[top++] = 0;
    visited[0] = 1;
    
    while (top > 0) {
        int v = stack[--top];
        
        for (int i = 0; i < job_cnt; i++) {
            for (int d = 0; d < jobs[i].deps_count; d++) {
                if (strcmp(jobs[i].deps[d], jobs[v].name) == 0 && !visited[i]) {
                    visited[i] = 1;
                    stack[top++] = i;
                }
            }
            for (int d = 0; d < jobs[v].deps_count; d++) {
                if (strcmp(jobs[v].deps[d], jobs[i].name) == 0 && !visited[i]) {
                    visited[i] = 1;
                    stack[top++] = i;
                }
            }
        }
    }
    
    for (int i = 0; i < job_cnt; i++) {
        if (!visited[i]) {
            printf("Error: multiple connected components\n");
            return 0;
        }
    }
    return 1;
}

int check_start_end() {
    int start = 0, end = 0;
    
    for (int i = 0; i < job_cnt; i++)
        if (jobs[i].deps_count == 0) start++;
    
    for (int i = 0; i < job_cnt; i++) {
        int is_end = 1;
        for (int j = 0; j < job_cnt; j++) {
            for (int d = 0; d < jobs[j].deps_count; d++) {
                if (strcmp(jobs[j].deps[d], jobs[i].name) == 0) {
                    is_end = 0;
                    break;
                }
            }
            if (!is_end) break;
        }
        if (is_end) end++;
    }
    
    if (start == 0) {
        printf("Error: no start jobs\n");
        return 0;
    }
    if (end == 0) {
        printf("Error: no end jobs\n");
        return 0;
    }
    return 1;
}

// ===================== ВЫПОЛНЕНИЕ =====================

void barrier_wait(Barrier* b) {
    pthread_mutex_lock(&b->mutex);
    b->done++;
    if (b->done < b->total) {
        pthread_cond_wait(&b->cond, &b->mutex);
    } else {
        pthread_cond_broadcast(&b->cond);
    }
    pthread_mutex_unlock(&b->mutex);
}

void* run_job(void* arg) {
    Job* job = (Job*)arg;
    
    pthread_mutex_lock(&lock);
    while (job->done_deps < job->deps_count && !failed)
        pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);
    
    if (failed) return NULL;
    
    printf("Start: %s\n", job->name);
    
    job->pid = fork();
    if (job->pid == 0) {
        execl("/bin/sh", "sh", "-c", job->command, NULL);
        exit(1);
    } else {
        int status;
        waitpid(job->pid, &status, 0);
        if (WEXITSTATUS(status) != 0) {
            printf("Error: %s\n", job->name);
            failed = 1;
            pthread_cond_broadcast(&cond);
            return NULL;
        }
    }
    
    printf("Done:  %s\n", job->name);
    
    if (strlen(job->barrier) > 0) {
        Barrier* b = find_barrier(job->barrier);
        if (b) barrier_wait(b);
    }
    
    pthread_mutex_lock(&lock);
    job->done = 1;
    
    for (int i = 0; i < job_cnt; i++)
        for (int d = 0; d < jobs[i].deps_count; d++)
            if (strcmp(jobs[i].deps[d], job->name) == 0)
                jobs[i].done_deps++;
    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
    
    return NULL;
}

// ===================== ПРОСТОЙ YAML ПАРСЕР =====================

void parse_simple_yaml(const char* filename) {
    printf("DEBUG: Parsing file: %s\n", filename);
    
    FILE* f = fopen(filename, "r");
    if (!f) { 
        perror("Error opening file");
        exit(1); 
    }
    
    char line[1024];
    int line_num = 0;
    int in_jobs = 0, in_job = 0, in_barriers = 0;
    char current_job[32] = "";
    int job_idx = -1;
    
    memset(jobs, 0, sizeof(jobs));
    memset(barriers, 0, sizeof(barriers));
    job_cnt = 0;
    barrier_cnt = 0;
    
    while (fgets(line, sizeof(line), f)) {
        line_num++;
        
        // Убираем перевод строки
        line[strcspn(line, "\n")] = 0;
        
        // Пропускаем комментарии и пустые строки
        if (line[0] == '#' || strlen(line) == 0) {
            continue;
        }
        
        // Убираем ВСЕ ведущие пробелы для анализа
        char* l = line;
        while (*l == ' ' || *l == '\t') l++;
        
        // Проверяем разделы
        if (strcmp(l, "jobs:") == 0) {
            in_jobs = 1;
            in_barriers = 0;
            in_job = 0;
            continue;
        } else if (strcmp(l, "barriers:") == 0) {
            in_jobs = 0;
            in_barriers = 1;
            in_job = 0;
            continue;
        }
        
        if (in_jobs) {
            // Проверяем начало новой задачи (имя заканчивается двоеточием, НЕ "command:", "deps:", "barrier:")
            if (strchr(l, ':') && 
                strstr(l, "command:") == NULL && 
                strstr(l, "deps:") == NULL && 
                strstr(l, "barrier:") == NULL) {
                
                // Это новая задача
                char* colon = strchr(l, ':');
                if (colon) {
                    *colon = '\0';
                    
                    // Убираем пробелы из имени
                    char* name = l;
                    while (*name == ' ') name++;
                    
                    printf("DEBUG: Found job: '%s'\n", name);
                    
                    strcpy(current_job, name);
                    job_idx = job_cnt;
                    strcpy(jobs[job_idx].name, name);
                    jobs[job_idx].deps_count = 0;
                    jobs[job_idx].done_deps = 0;
                    jobs[job_idx].done = 0;
                    jobs[job_idx].barrier[0] = '\0';
                    strcpy(jobs[job_idx].command, "echo 'default'");
                    job_cnt++;
                    in_job = 1;
                }
            }
            // Параметры задачи (содержат "command:", "deps:", "barrier:")
            else if (in_job && job_idx >= 0) {
                if (strstr(l, "command:") == l) {
                    char* value = l + 8; // после "command:"
                    while (*value == ' ') value++;
                    strcpy(jobs[job_idx].command, value);
                    printf("DEBUG: Job '%s' command: '%s'\n", current_job, value);
                }
                else if (strstr(l, "barrier:") == l) {
                    char* value = l + 8; // после "barrier:"
                    while (*value == ' ') value++;
                    strcpy(jobs[job_idx].barrier, value);
                    printf("DEBUG: Job '%s' barrier: '%s'\n", current_job, value);
                }
                else if (strstr(l, "deps:") == l) {
                    char* value = l + 5; // после "deps:"
                    while (*value == ' ') value++;
                    
                    // Обработка зависимостей
                    if (strcmp(value, "[]") == 0) {
                        // Пустой список
                        jobs[job_idx].deps_count = 0;
                    } else {
                        // Убираем скобки
                        if (value[0] == '[') value++;
                        char* end = value + strlen(value) - 1;
                        if (*end == ']') *end = '\0';
                        
                        // Разделяем по запятым
                        char* token = strtok(value, ",");
                        while (token && jobs[job_idx].deps_count < MAX_DEPS) {
                            // Убираем пробелы и кавычки
                            while (*token == ' ' || *token == '"' || *token == '\'') token++;
                            char* t = token + strlen(token) - 1;
                            while (t > token && (*t == ' ' || *t == '"' || *t == '\'')) *t-- = '\0';
                            
                            if (strlen(token) > 0) {
                                strcpy(jobs[job_idx].deps[jobs[job_idx].deps_count], token);
                                jobs[job_idx].deps_count++;
                                printf("DEBUG: Job '%s' depends on: '%s'\n", current_job, token);
                            }
                            token = strtok(NULL, ",");
                        }
                    }
                }
            }
        }
        else if (in_barriers) {
            // Барьер (имя с двоеточием, но не "jobs:")
            if (strchr(l, ':') && strstr(l, "jobs:") == NULL) {
                char* colon = strchr(l, ':');
                if (colon) {
                    *colon = '\0';
                    
                    char* name = l;
                    while (*name == ' ') name++;
                    
                    printf("DEBUG: Found barrier: '%s'\n", name);
                    
                    Barrier* barrier = &barriers[barrier_cnt];
                    strcpy(barrier->name, name);
                    barrier->total = 0;
                    barrier->done = 0;
                    pthread_mutex_init(&barrier->mutex, NULL);
                    pthread_cond_init(&barrier->cond, NULL);
                    barrier_cnt++;
                }
            }
            // Задачи барьера
            else if (barrier_cnt > 0 && strstr(l, "jobs:") == l) {
                char* jobs_list = l + 5; // после "jobs:"
                while (*jobs_list == ' ') jobs_list++;
                
                printf("DEBUG: Barrier jobs list: '%s'\n", jobs_list);
                
                // Убираем скобки
                if (jobs_list[0] == '[') jobs_list++;
                char* end = jobs_list + strlen(jobs_list) - 1;
                if (*end == ']') *end = '\0';
                
                // Разделяем задачи
                char* token = strtok(jobs_list, ",");
                while (token) {
                    while (*token == ' ' || *token == '"' || *token == '\'') token++;
                    char* t = token + strlen(token) - 1;
                    while (t > token && (*t == ' ' || *t == '"' || *t == '\'')) *t-- = '\0';
                    
                    if (strlen(token) > 0) {
                        barriers[barrier_cnt-1].total++;
                        printf("DEBUG: Barrier job: '%s'\n", token);
                    }
                    token = strtok(NULL, ",");
                }
            }
        }
    }
    
    fclose(f);
    printf("DEBUG: Finished parsing. Found %d jobs, %d barriers\n", job_cnt, barrier_cnt);
}
// ===================== ГЛАВНАЯ ФУНКЦИЯ =====================

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <config.yaml>\n", argv[0]);
        return 1;
    }
    
    // Читаем YAML конфиг
    parse_simple_yaml(argv[1]);
    
    // Проверяем DAG
    if (!check_cycles()) return 1;
    if (!check_connected()) return 1;
    if (!check_start_end()) return 1;
    
    printf("\nDAG is valid. Starting execution...\n\n");
    
    // Запускаем задачи
    for (int i = 0; i < job_cnt; i++)
        pthread_create(&jobs[i].thread, NULL, run_job, &jobs[i]);
    
    // Ждём завершения
    for (int i = 0; i < job_cnt; i++)
        pthread_join(jobs[i].thread, NULL);
    
    if (failed) {
        printf("\nDAG failed\n");
        return 1;
    }
    
    printf("\nAll jobs done successfully\n");
    return 0;
}