#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

typedef struct
{
    char *buffer;
    uint64_t buffer_length;
    int64_t input_length;
} input_buffer_t;

typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND,
} meta_command_result_t;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT,
} prepare_result_t;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} statement_type_t;

typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
} execute_result_t;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct
{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} row_t;

typedef struct
{
    statement_type_t type;
    row_t row_to_insert;
} statement_t;

#define TABLE_MAX_PAGES 100
typedef struct
{
    FILE *file;
    uint32_t file_length;
    void *pages[TABLE_MAX_PAGES];
} pager_t;

typedef struct
{
    pager_t *pager;
    uint32_t num_rows;
} table_t;

const uint32_t ID_SIZE = size_of_attribute(row_t, id);
const uint32_t USERNAME_SIZE = size_of_attribute(row_t, username);
const uint32_t EMAIL_SIZE = size_of_attribute(row_t, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

int64_t getline(char **lineptr, int64_t *n, FILE *stream)
{
    char *buf_ptr = NULL;
    char *p = buf_ptr;
    int64_t size;
    int32_t c;

    if (lineptr == NULL || stream == NULL || n == NULL)
    {
        return -1;
    }

    buf_ptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF)
    {
        return -1;
    }

    if (buf_ptr == NULL)
    {
        buf_ptr = malloc(128);
        if (buf_ptr == NULL)
        {
            return -1;
        }
        size = 128;
    }

    p = buf_ptr;
    while (c != EOF)
    {
        if ((p - buf_ptr) > (size - 1))
        {
            size = size + 128;
            buf_ptr = realloc(buf_ptr, size);
            if (buf_ptr == NULL)
            {
                return -1;
            }
            p = buf_ptr + (size - 128);
        }
        *p++ = c;
        if (c == '\n')
        {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = buf_ptr;
    *n = size;

    return p - buf_ptr - 1;
}

input_buffer_t *create_input_buffer()
{
    input_buffer_t *input_buffer = malloc(sizeof(input_buffer_t));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void close_input_buffer(input_buffer_t *input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}

pager_t *pager_open(const char *filename)
{
    FILE *file = fopen(filename, "r+t");
    if (file == NULL)
    {
        file = fopen(filename, "w+t");
        if (file == NULL)
        {
            printf("Unable to open file\n");
            exit(EXIT_FAILURE);
        }
    }

    int32_t result = fseek(file, 0, SEEK_END);
    if (result)
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    off_t file_length = ftell(file);
    if (file_length < 0)
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    pager_t *pager = malloc(sizeof(pager_t));
    pager->file = file;
    pager->file_length = file_length;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
    {
        pager->pages[i] = NULL;
    }

    return pager;
}

void *get_page(pager_t *pager, uint32_t page_num)
{
    if (page_num > TABLE_MAX_PAGES)
    {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL)
    {
        void *page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;

        if (pager->file_length % PAGE_SIZE)
        {
            num_pages += 1;
        }

        if (page_num <= num_pages)
        {
            fseek(pager->file, page_num * PAGE_SIZE, SEEK_SET);
            int64_t bytes_read = fread(page, sizeof(uint8_t), PAGE_SIZE, pager->file);
            if (bytes_read < 0 || ferror(pager->file))
            {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[page_num] = page;
    }

    return pager->pages[page_num];
}

void pager_flush(pager_t *pager, uint32_t page_num, uint32_t size)
{
    if (pager->pages[page_num] == NULL)
    {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = fseek(pager->file, page_num * PAGE_SIZE, SEEK_SET);
    if (offset)
    {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    int64_t bytes_written = fwrite(pager->pages[page_num], sizeof(uint8_t), size, pager->file);
    if (bytes_written < size)
    {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

void print_prompt()
{
    printf("db > ");
}

void read_input(input_buffer_t *input_buffer)
{
    int64_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0';
}

void serialize_row(row_t *source, void *destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    strncpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    strncpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, row_t *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void *row_slot(table_t *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = get_page(table->pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

void print_row(row_t *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

table_t *db_open(const char *filename)
{
    pager_t *pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;

    table_t *table = malloc(sizeof(table_t));
    table->pager = pager;
    table->num_rows = num_rows;

    return table;
}

void db_close(table_t *table)
{
    pager_t *pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < num_full_pages; ++i)
    {
        if (pager->pages[i] == NULL)
        {
            continue;
        }

        pager_flush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0)
    {
        uint32_t page_num = num_full_pages;
        if (pager->pages[page_num] != NULL)
        {
            pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }

    int32_t result = fclose(pager->file);
    if (result)
    {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i)
    {
        void *page = pager->pages[i];
        if (page)
        {
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free(pager);
    free(table);
}

meta_command_result_t do_meta_command(input_buffer_t *input_buffer, table_t *table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

prepare_result_t prepare_insert(input_buffer_t *input_buffer, statement_t *statement)
{
    statement->type = STATEMENT_INSERT;

    char *keyword = strtok(input_buffer->buffer, " ");
    char *id_string = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (id_string == NULL || username == NULL || email == NULL)
    {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(id_string);
    if (id < 0)
    {
        return PREPARE_NEGATIVE_ID;
    }

    if (strlen(username) > COLUMN_USERNAME_SIZE)
    {
        return PREPARE_STRING_TOO_LONG;
    }

    if (strlen(email) > COLUMN_EMAIL_SIZE)
    {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.username, username);
    strcpy(statement->row_to_insert.email, email);

    return PREPARE_SUCCESS;
}

prepare_result_t prepare_statement(input_buffer_t *input_buffer, statement_t *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
        return prepare_insert(input_buffer, statement);
    }

    if (strcmp(input_buffer->buffer, "select") == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

execute_result_t execute_insert(statement_t *statement, table_t *table)
{
    if (table->num_rows >= TABLE_MAX_ROWS)
    {
        return EXECUTE_TABLE_FULL;
    }

    row_t *row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

execute_result_t execute_select(statement_t *statement, table_t *table)
{
    row_t row;
    for (int32_t i = 0; i < table->num_rows; ++i)
    {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}

execute_result_t execute_statement(statement_t *statement, table_t *table)
{
    switch (statement->type)
    {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    case STATEMENT_SELECT:
        return execute_select(statement, table);
    }
}

int32_t main(int32_t argc, char **argv)
{
    if (argc < 2)
    {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    table_t *table = db_open(filename);
    input_buffer_t *input_buffer = create_input_buffer();

    while (1)
    {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case META_COMMAND_SUCCESS:
                continue;
            case META_COMMAND_UNRECOGNIZED_COMMAND:
                printf("Unrecognized command '%s'\n", input_buffer->buffer);
                continue;
            }
        }

        statement_t statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case PREPARE_SUCCESS:
            break;
        case PREPARE_NEGATIVE_ID:
            printf("ID must be positive.\n");
            continue;
        case PREPARE_STRING_TOO_LONG:
            printf("String is too long.\n");
            continue;
        case PREPARE_SYNTAX_ERROR:
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
            continue;
        }

        switch (execute_statement(&statement, table))
        {
        case EXECUTE_SUCCESS:
            printf("Executed.\n");
            break;
        case EXECUTE_TABLE_FULL:
            printf("Error: Table full.\n");
            break;
        }
    }

    return 0;
}