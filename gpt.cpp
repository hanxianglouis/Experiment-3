#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS 30
#define COLS 50

// 定义三元组结构体
struct Triple {
    int row;
    int col;
    char title[100];
};

// 定义压缩矩阵结构体
struct CompressedMatrix {
    struct Triple data[150];
    int rpos[ROWS];
    int mu, nu, tu;
};

// 读取CSV文件并将数据存储到二维字符数组中
void file_to_matrix(char a[ROWS][COLS][100]) {
    FILE *file = fopen("FirmPatent.csv", "r");
    if (!file) {
        printf("Failed to open file.\n");
        return;
    }
    
    char line[1024];
    fgets(line, sizeof(line), file); // Skip header line

    while (fgets(line, sizeof(line), file)) {
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '"') {
                memmove(&line[i], &line[i + 1], strlen(line) - i);
                i--; // Adjust index to process consecutive quotes
            }
        }

        int patentid, firmid[3] = {0};
        char title[100];
        char *token = strtok(line, ",");

        patentid = atoi(token);
        token = strtok(NULL, ",");
        strcpy(title, token);

        for (int i = 0; i < 3 && (token = strtok(NULL, ",")) != NULL; i++) {
            firmid[i] = atoi(token);
        }

        for (int i = 0; i < 3 && firmid[i] != 0; i++) {
            strcpy(a[firmid[i] - 1001][patentid - 100001], title);
        }
    }

    fclose(file);
}

// 初始化压缩矩阵
void init_compressed_matrix(struct CompressedMatrix *cm, char a[ROWS][COLS][100]) {
    cm->mu = ROWS;
    cm->nu = COLS;
    cm->tu = 0;
    for (int i = 0; i < ROWS; i++) {
        cm->rpos[i] = cm->tu;
        for (int j = 0; j < COLS; j++) {
            if (a[i][j][0] != '\0') {
                cm->data[cm->tu].row = i;
                cm->data[cm->tu].col = j;
                strcpy(cm->data[cm->tu].title, a[i][j]);
                cm->tu++;
            }
        }
    }
}

// 转置压缩矩阵
void transpose_matrix(const struct CompressedMatrix *cm, struct CompressedMatrix *tm) {
    tm->mu = cm->nu;
    tm->nu = cm->mu;
    tm->tu = cm->tu;

    int num[COLS] = {0};
    int cpot[COLS] = {0};

    for (int i = 0; i < cm->tu; i++) {
        num[cm->data[i].col]++;
    }
    cpot[0] = 0;
    for (int i = 1; i < cm->nu; i++) {
        cpot[i] = cpot[i - 1] + num[i - 1];
    }
    for (int i = 0; i < cm->tu; i++) {
        int col = cm->data[i].col;
        int loc = cpot[col];
        tm->data[loc].row = cm->data[i].col;
        tm->data[loc].col = cm->data[i].row;
        strcpy(tm->data[loc].title, cm->data[i].title);
        cpot[col]++;
    }
}

// 打印压缩矩阵
void print_compressed_matrix(const struct CompressedMatrix *cm) {
    printf("Row\tCol\tTitle\n");
    for (int i = 0; i < cm->tu; i++) {
        printf("%d\t%d\t%s\n", cm->data[i].row, cm->data[i].col, cm->data[i].title);
    }
}

// KMP算法 - 获取next数组
int* get_next(char *t) {
    int len = strlen(t);
    int *next = (int*)malloc(len * sizeof(int));
    next[0] = -1;
    for (int i = 1; i < len; i++) {
        int j = next[i - 1];
        while (j >= 0 && t[j + 1] != t[i]) {
            j = next[j];
        }
        next[i] = (t[j + 1] == t[i]) ? j + 1 : -1;
    }
    return next;
}

// KMP算法 - 模式匹配函数
int pattern_match_kmp(char *s, char *t, int *next) {
    int n = strlen(s);
    int m = strlen(t);
    int p = -1, i = 0, j = 0;
    while (j < n && i < m) {
        if (s[j] == t[i]) {
            i++;
            j++;
        } else if (i > 0) {
            i = next[i - 1] + 1;
        } else {
            j++;
        }
    }
    if (i == m) {
        p = j - m;
    }
    return p;
}

// 菜单功能
void menu(struct CompressedMatrix *cm) {
    int choice;
    do {
        printf("\nMenu:\n");
        printf("1. 查找专利\n");
        printf("2. 显示转置矩阵三元组\n");
        printf("3. 计算公司共享专利数量\n");
        printf("0. 退出\n");
        printf("选择一个选项: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                char patent_title[100];
                printf("输入要查找的专利名称: ");
                scanf("%s", patent_title);
                int *next = get_next(patent_title);
                int found = 0;
                for (int i = 0; i < cm->tu; i++) {
                    if (pattern_match_kmp(cm->data[i].title, patent_title, next) != -1) {
                        printf("专利找到: Row %d, Col %d, Title %s\n",
                               cm->data[i].row, cm->data[i].col, cm->data[i].title);
                        found = 1;
                    }
                }
                if (!found) {
                    printf("未找到匹配的专利。\n");
                }
                free(next);
                break;
            }
            case 2: {
                struct CompressedMatrix tm;
                transpose_matrix(cm, &tm);
                printf("转置矩阵三元组:\n");
                print_compressed_matrix(&tm);
                break;
            }
            case 3: {
                int company1, company2, shared_count = 0;
                printf("输入公司1 ID: ");
                scanf("%d", &company1);
                printf("输入公司2 ID: ");
                scanf("%d", &company2);

                for (int i = 0; i < COLS; i++) {
                    if (strcmp(cm->data[company1].title, cm->data[company2].title) == 0) {
                        shared_count++;
                    }
                }
                printf("共享专利数量: %d\n", shared_count);
                break;
            }
            case 0:
                printf("退出程序。\n");
                break;
            default:
                printf("无效选项，请重新选择。\n");
        }
    } while (choice != 0);
}

// 主函数
int main() {
    char matrix[ROWS][COLS][100] = {0};
    struct CompressedMatrix cm;

    // 从文件中读取矩阵数据
    file_to_matrix(matrix);

    // 初始化压缩矩阵
    init_compressed_matrix(&cm, matrix);

    // 调用菜单功能
    menu(&cm);

    return 0;
}
