//project 1 of COMP30023
//1312068， Yicheng Zhao

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int present;
    int frame;
    int page;
    unsigned long timestamp;
} tlb_entry_t;

// Extract page number and offset from logical address
void extract_logical_parts(int input, int *page, int *offset) {
    *offset = input & 0b111111111111;
    input = input >> 12;
    *page = input & 0b111111111111;
}

// Search page table or TLB
int search_table(int pagenum, tlb_entry_t table[], int size) {
    for (int i = 0; i < size; i++) {
        if (table[i].page == pagenum && table[i].present == 1) {
            return i;
        }
    }
    return -1;
}

// Check if TLB is full
int check_tlb_full(tlb_entry_t TLB[]) {
    int used = 0;
    for (int i = 0; i < 32; i++) {
        if (TLB[i].present == 1) {
            used++;
        }
    }
    return used == 32;
}

// Select least recently used TLB entry
int select_lru_entry(tlb_entry_t TLB[]) {
    int idx = 0;
    int min_time = INT_MAX;
    for (int i = 0; i < 32; i++) {
        if (TLB[i].present == 1 && TLB[i].timestamp < min_time) {
            min_time = TLB[i].timestamp;
            idx = i;
        }
    }
    return idx;
}

// Process TLB hit
void process_tlb_hit(tlb_entry_t *entry, int page, int offset, unsigned long *current_time) {
    printf("tlb-hit=1,page-number=%d,frame=%d,physical-address=%d\n",
           page, entry->frame, entry->frame * 4096 + offset);
    entry->timestamp = *current_time;
    (*current_time)++;
}

// Invalidate a TLB entry
void invalidate_tlb_entry(tlb_entry_t TLB[], int old_page) {
    for (int i = 0; i < 32; i++) {
        if (TLB[i].page == old_page && TLB[i].present == 1) {
            TLB[i].present = 0;
            int valid = 0;
            for (int j = 0; j < 32; j++) {
                if (TLB[j].present == 1) {
                    valid++;
                }
            }
            printf("tlb-flush=%d,tlb-size=%d\n", old_page, valid);
            break;
        }
    }
}

// Update or insert into TLB
void update_tlb_entry(tlb_entry_t TLB[], int pagenum, int frame_num, unsigned long current_time) {
    int insert_pos = 0;
    if (check_tlb_full(TLB)) {
        insert_pos = select_lru_entry(TLB);
        printf("tlb-remove=%d,tlb-add=%d\n", TLB[insert_pos].page, pagenum);
    } else {
        for (int i = 0; i < 32; i++) {
            if (TLB[i].present == 0) {
                insert_pos = i;
                printf("tlb-remove=none,tlb-add=%d\n", pagenum);
                break;
            }
        }
    }
    TLB[insert_pos].page = pagenum;
    TLB[insert_pos].frame = frame_num;
    TLB[insert_pos].present = 1;
    TLB[insert_pos].timestamp = current_time;
}

// Task 1
void run_task1(FILE *fp) {
    int input;
    while (fscanf(fp, "%d", &input) == 1) {
        int pagenum, offset;
        extract_logical_parts(input, &pagenum, &offset);
        printf("logical-address=%d,page-number=%d,offset=%d\n", input, pagenum, offset);
    }
}

// Task 2
void run_task2(FILE *fp) {
    int input;
    tlb_entry_t page_table[1024] = {0};
    int frame_count = 0;
    int page_count = 0;

    while (fscanf(fp, "%d", &input) == 1) {
        int pagenum, offset;
        extract_logical_parts(input, &pagenum, &offset);
        printf("logical-address=%d,page-number=%d,offset=%d\n", input, pagenum, offset);

        int idx = search_table(pagenum, page_table, page_count);
        if (idx != -1) {
            printf("page-number=%d,page-fault=0,frame-number=%d,physical-address=%d\n",
                   pagenum, page_table[idx].frame, offset + 4096 * page_table[idx].frame);
        } else {
            page_table[page_count].page = pagenum;
            page_table[page_count].frame = frame_count;
            page_table[page_count].present = 1;
            printf("page-number=%d,page-fault=1,frame-number=%d,physical-address=%d\n",
                   pagenum, frame_count, offset + 4096 * frame_count);
            frame_count++;
            page_count++;
        }
    }
}

// Task 3
void run_task3(FILE *fp) {
    int input;
    tlb_entry_t page_table[1024] = {0};
    int frame_slots[256];
    for (int i = 0; i < 256; i++) frame_slots[i] = -1;
    int frame_count = 0;
    int page_count = 0;

    while (fscanf(fp, "%d", &input) == 1) {
        int pagenum, offset;
        extract_logical_parts(input, &pagenum, &offset);
        printf("logical-address=%d,page-number=%d,offset=%d\n", input, pagenum, offset);

        int idx = search_table(pagenum, page_table, page_count);
        if (idx != -1) {
            printf("page-number=%d,page-fault=0,frame-number=%d,physical-address=%d\n",
                   pagenum, page_table[idx].frame, offset + 4096 * page_table[idx].frame);
        } else {
            if (frame_slots[frame_count] != -1) {
                printf("evicted-page=%d,freed-frame=%d\n", frame_slots[frame_count], frame_count);
                for (int j = 0; j < page_count; j++) {
                    if (page_table[j].page == frame_slots[frame_count]) {
                        page_table[j].present = 0;
                        break;
                    }
                }
            }
            frame_slots[frame_count] = pagenum;
            page_table[page_count].page = pagenum;
            page_table[page_count].frame = frame_count;
            page_table[page_count].present = 1;
            printf("page-number=%d,page-fault=1,frame-number=%d,physical-address=%d\n",
                   pagenum, frame_count, offset + 4096 * frame_count);

            frame_count++;
            if (frame_count == 256) frame_count = 0;
            page_count++;
        }
    }
}

// Task 4
void run_task4(FILE *fp) {
    int input;
    tlb_entry_t page_table[1024] = {0};
    int frame_slots[256];
    for (int i = 0; i < 256; i++) frame_slots[i] = -1;
    int frame_count = 0;
    int page_count = 0;
    unsigned long current_time = 0;
    tlb_entry_t TLB[32] = {0};

    while (fscanf(fp, "%d", &input) == 1) {
        int pagenum, offset;
        extract_logical_parts(input, &pagenum, &offset);
        printf("logical-address=%d,page-number=%d,offset=%d\n", input, pagenum, offset);

        int tlb_idx = search_table(pagenum, TLB, 32);
        if (tlb_idx != -1) {
            process_tlb_hit(&TLB[tlb_idx], pagenum, offset, &current_time);
            continue;
        } else {
            printf("tlb-hit=0,page-number=%d,frame=none,physical-address=none\n", pagenum);
        }

        int idx = search_table(pagenum, page_table, page_count);
        if (idx != -1) {
            printf("page-number=%d,page-fault=0,frame-number=%d,physical-address=%d\n",
                   pagenum, page_table[idx].frame, offset + 4096 * page_table[idx].frame);
        } else {
            if (frame_slots[frame_count] != -1) {
                printf("evicted-page=%d,freed-frame=%d\n", frame_slots[frame_count], frame_count);
                invalidate_tlb_entry(TLB, frame_slots[frame_count]);
                for (int j = 0; j < page_count; j++) {
                    if (page_table[j].page == frame_slots[frame_count]) {
                        page_table[j].present = 0;
                        break;
                    }
                }
            }
            frame_slots[frame_count] = pagenum;
            page_table[page_count].page = pagenum;
            page_table[page_count].frame = frame_count;
            page_table[page_count].present = 1;
            printf("page-number=%d,page-fault=1,frame-number=%d,physical-address=%d\n",
                   pagenum, frame_count, offset + 4096 * frame_count);

            update_tlb_entry(TLB, pagenum, frame_count, current_time);

            frame_count++;
            if (frame_count == 256) frame_count = 0;
            page_count++;
        }
        current_time++;
    }
}

int main(int argc, char *argv[]) {
    char *filename = NULL;
    char *task = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && (i + 1) < argc) {
            filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-t") == 0 && (i + 1) < argc) {
            task = argv[i + 1];
            i++;
        } else {
            fprintf(stderr, "Invalid argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (filename == NULL || task == NULL) {
        fprintf(stderr, "Usage: %s -f <filename> -t <task>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Failed to open input file");
        return 1;
    }

    if (strcmp(task, "task1") == 0) {
        run_task1(fp);
    } else if (strcmp(task, "task2") == 0) {
        run_task2(fp);
    } else if (strcmp(task, "task3") == 0) {
        run_task3(fp);
    } else if (strcmp(task, "task4") == 0) {
        run_task4(fp);
    } else {
        fprintf(stderr, "Unsupported task: %s\n", task);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
