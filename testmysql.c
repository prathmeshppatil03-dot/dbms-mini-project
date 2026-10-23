#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#define MAX 100

typedef struct
{
    int id;
    char name[50];
    char category[50];
    int quantity;
    float price;
    int supplier_id;
} Product;

typedef struct
{
    int id;
    char name[50];
    char contact[20];
    char email[80];
} Supplier;

typedef struct
{
    int quotation_id;
    int supplier_id;
    int product_id;
    int quantity;
    float price;
} Quotation;

MYSQL *conn;

void line()
{
    printf("+----------------------------------------------------------------------------+\n");
}

void title(const char *text)
{
    printf("\n");
    line();
    printf("| %-74s |\n", text);
    line();
}

void pauseScreen()
{
    printf("\nPress ENTER to continue...");
    getchar();
    getchar();
}

void successMessage(const char *message)
{
    printf("\n[SUCCESS] %s\n", message);
}

void errorMessage(const char *message)
{
    printf("\n[ERROR] %s\n", message);
}

void connectDatabase()
{
    conn = mysql_init(NULL);

    if (conn == NULL)
    {
        printf("\nMySQL initialization failed.\n");
        exit(1);
    }

    if (mysql_real_connect(
            conn,
            "localhost",
            "root",
            "7841816785",
            "smart_procurement",
            3306,
            NULL,
            0) == NULL)
    {
        printf("\nDatabase connection failed:\n%s\n",
               mysql_error(conn));

        mysql_close(conn);
        exit(1);
    }

    printf("\n");
    printf("+------------------------------------------------------+\n");
    printf("|       DATABASE CONNECTION SUCCESSFUL                 |\n");
