#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
// Added for debugging
#include <inttypes.h>

#include "parse.h"
#include "common.h"
#include "file.h"


void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    int i = 0;
    for (; i < dbhdr->count; i++) {
        printf("Employee:%d\n", i);
        printf("\tName:%s\n", employees[i].name);
        printf("\tAddress:%s\n", employees[i].address);
        printf("\tHours:%d\n", employees[i].hours);
    }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {
    printf("%s\n", addstring);

    char *name = strtok(addstring, ",");

    char *addr = strtok(NULL, ",");

    char *hours = strtok(NULL, ",");

    printf("%s %s %s\n", name, addr, hours);

    strncpy(employees[dbhdr->count-1].name, name, sizeof(employees[dbhdr->count-1].name));

    strncpy(employees[dbhdr->count-1].address, addr, sizeof(employees[dbhdr->count-1].address));

    employees[dbhdr->count-1].hours = atoi(hours);

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }


    int count = dbhdr->count;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == -1) {
        printf("Malloc failed\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count*sizeof(struct employee_t));

    int i = 0;
    for (; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}
	

void output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    int realcount = dbhdr->count;

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));
    // Removed for debugging
    // dbhdr->count = htons(dbhdr->count);
    // dbhdr->version = htons(dbhdr->version);


    printf("Middle of output_file function FD:%d\n", fd);
    
    lseek(fd, 0, SEEK_SET);

    write(fd, dbhdr, sizeof(struct dbheader_t));

    int i = 0;
    for (; i < realcount; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return;
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if(fd < 0) {
        printf("Got a bad FD from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if(header == NULL) {
        printf("Malloc failed to return database header\n");
        return STATUS_ERROR;
    }

    if(read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        printf("validate_db_header struct size%zu\n", sizeof(struct dbheader_t));
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->magic = ntohl(header->magic);
    header->filesize = ntohl(header->filesize);

    // Debug check before network btye conversion
    printf("Version before ntohs: %u\n", header->version);

    header->version = ntohs(header->version);

    // Debug check after network byte conversion
    printf("Version after ntohs: %u\n", header->version);
    printf("Size of dbheader_t: %zu\n", sizeof(struct dbheader_t));

    header->count = ntohs(header->count);


    if(header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return -1;
    }

    if(header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return -1;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    // Added for debugging
    printf("dbstat.st_size %" PRId64 "\n", (int64_t)dbstat.st_size);

    if(header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return -1;
    }

    *headerOut = header;

    // Not in original source code
    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if(header == NULL) {
        printf("Malloc failed to print dbheader\n");
        return STATUS_ERROR;
    }

    printf("Middle of create_db_header:%d\n", fd);


    // Removed for deubgging
    // header->version = 0x1;
    header->version = htons(0x1);
    header->count = 0;
    // Removed for debugging
    // header->magic = HEADER_MAGIC;
    header->magic = htonl(HEADER_MAGIC);
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek");
        free(header);
        return STATUS_ERROR;
    }

    if (write(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("write");
        free(header);
        return STATUS_ERROR;
    }

    printf("Create db_header structure values:\n");
    printf("Version:%d\n", header->version);
    printf("Count:%d\n", header->count);
    printf("Magic Number:0X%0x\n",header->magic);
    printf("Filesize:%d\n", header->filesize);

    printf("Create db header function completed successfully %d\n", STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

void find_employee(int fd, struct dbheader_t *dbhdr, struct employee_t *employees, const char *name) {
    if (fd < 0 || dbhdr->count == 0) {
        printf("Invalid file or no employees found.\n");
        return;
    }    

    
    for (int i = 0; i < dbhdr->count; i++) {
        if (strcmp(employees[i].name, name) == 0) {
            printf("Employee Found:\n");
            printf("Name: %s\n", employees[i].name);
            printf("Address: %s\n", employees[i].address);
            printf("Hours: %d\n", employees[i].hours);

            // Get new hours from user input
            int new_hours;
            printf("Enter new hours for the employee: ");
            if (scanf("%d", &new_hours) != 1) {
                printf("Invalid input.\n");
                return;
            }

            // Converts new_hours to network byte order
            new_hours = htonl(new_hours);

            // Update the employee's hours in memory
            employees[i].hours = new_hours;

            // Calculate the offset of the employee record in the file
            off_t offset = sizeof(struct dbheader_t) + i * sizeof(struct employee_t);

            // Seek to the appropriate position in the file
            if (lseek(fd, offset, SEEK_SET) == -1) {
                perror("lseek");
                return;
            }

            // Write the updated employee record to the file
            if (write(fd, &employees[i], sizeof(struct employee_t)) != sizeof(struct employee_t)) {
                perror("write");
                return;
            }

            printf("Hours updated to: %d\n", ntohl(new_hours));
            return;
        }
    }

    printf("Employee not found.\n");
}

