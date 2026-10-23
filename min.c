#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

#define MAX 100

/* =====================================================
   TERMINAL COLORS
   ===================================================== */

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"
#define MAGENTA "\033[1;35m"
#define GRAY    "\033[0;37m"

#define CLEAR "\033[2J\033[H"


/* =====================================================
   PRODUCT STRUCTURE
   ===================================================== */

typedef struct
{
    int id;
    char name[50];
    char category[50];
    int quantity;
    float price;
    int supplier_id;
} Product;


/* =====================================================
   SUPPLIER STRUCTURE
   ===================================================== */

typedef struct
{
    int id;
    char name[50];
    char contact[20];
    char email[80];
} Supplier;


/* =====================================================
   QUOTATION STRUCTURE
   ===================================================== */

typedef struct
{
    int quotation_id;
    int supplier_id;
    int product_id;
    int quantity;
    float price;
} Quotation;


/* =====================================================
   GLOBAL MYSQL CONNECTION
   ===================================================== */

MYSQL *conn;


/* =====================================================
   COMMON TERMINAL FUNCTIONS
   ===================================================== */

void line()
{
    printf(CYAN
           "+----------------------------------------------------------------------------+\n"
           RESET);
}


void title(const char *text)
{
    printf("\n");
    line();

    printf(CYAN "|");
    printf(WHITE " %-74s " RESET, text);
    printf(CYAN "|\n" RESET);

    line();
}


void pauseScreen()
{
    printf("\n");
    printf(GRAY "Press ENTER to continue..." RESET);

    getchar();
    getchar();
}


void successMessage(const char *message)
{
    printf("\n");
    printf(GREEN "  [SUCCESS] %s\n" RESET, message);
}


void errorMessage(const char *message)
{
    printf("\n");
    printf(RED "  [ERROR] %s\n" RESET, message);
}


/* =====================================================
   MYSQL CONNECTION
   ===================================================== */

void connectDatabase()
{
    conn = mysql_init(NULL);

    if (conn == NULL)
    {
        printf(RED
               "\nMySQL initialization failed.\n"
               RESET);

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
        printf(RED
               "\nDatabase connection failed:\n%s\n"
               RESET,
               mysql_error(conn));

        mysql_close(conn);
        exit(1);
    }

    printf("\n");

    printf(GREEN
           "  +------------------------------------------------------+\n"
           "  |       DATABASE CONNECTION SUCCESSFUL                 |\n"
           "  +------------------------------------------------------+\n"
           RESET);
}


/* =====================================================
   CHECK PRODUCT ID
   ===================================================== */

int productExists(int id)
{
    char query[200];
    MYSQL_RES *result;

    sprintf(query,
            "SELECT Product_ID FROM Products WHERE Product_ID = %d",
            id);

    if (mysql_query(conn, query))
        return 0;

    result = mysql_store_result(conn);

    if (result == NULL)
        return 0;

    int exists = (mysql_num_rows(result) > 0);

    mysql_free_result(result);

    return exists;
}


/* =====================================================
   CHECK SUPPLIER ID
   ===================================================== */

int supplierExists(int id)
{
    char query[200];
    MYSQL_RES *result;

    sprintf(query,
            "SELECT Supplier_ID FROM Suppliers WHERE Supplier_ID = %d",
            id);

    if (mysql_query(conn, query))
        return 0;

    result = mysql_store_result(conn);

    if (result == NULL)
        return 0;

    int exists = (mysql_num_rows(result) > 0);

    mysql_free_result(result);

    return exists;
}


/* =====================================================
   1. ADD PRODUCT
   ===================================================== */

void addProduct()
{
    Product p;

    char query[600];

    title("ADD NEW PRODUCT");

    printf(YELLOW " Product ID       : " RESET);
    scanf("%d", &p.id);

    if (productExists(p.id))
    {
        errorMessage("Product ID already exists.");
        return;
    }

    printf(YELLOW " Product Name     : " RESET);
    scanf(" %[^\n]", p.name);

    printf(YELLOW " Category         : " RESET);
    scanf(" %[^\n]", p.category);

    printf(YELLOW " Quantity         : " RESET);
    scanf("%d", &p.quantity);

    printf(YELLOW " Unit Price       : " RESET);
    scanf("%f", &p.price);

    printf(YELLOW " Supplier ID      : " RESET);
    scanf("%d", &p.supplier_id);

    if (!supplierExists(p.supplier_id))
    {
        errorMessage("Supplier ID does not exist.");

        printf(GRAY
               " Please add the supplier first or enter a valid Supplier ID.\n"
               RESET);

        return;
    }

    sprintf(query,
            "INSERT INTO Products "
            "(Product_ID, Product_Name, Category, Quantity, Unit_Price, Supplier_ID) "
            "VALUES (%d, '%s', '%s', %d, %.2f, %d)",
            p.id,
            p.name,
            p.category,
            p.quantity,
            p.price,
            p.supplier_id);

    if (mysql_query(conn, query))
    {
        printf(RED
               "\n Database Error: %s\n"
               RESET,
               mysql_error(conn));

        return;
    }

    successMessage("Product added successfully.");
}


/* =====================================================
   2. VIEW PRODUCTS
   ===================================================== */

void viewProducts()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("PRODUCT LIST");

    if (mysql_query(conn,
                    "SELECT Product_ID, Product_Name, Category, "
                    "Quantity, Unit_Price, Supplier_ID "
                    "FROM Products"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to retrieve products.");
        return;
    }

    printf(CYAN
           "\n %-8s %-25s %-18s %-10s %-12s %-12s\n"
           RESET,
           "ID",
           "PRODUCT",
           "CATEGORY",
           "QUANTITY",
           "PRICE",
           "SUPPLIER");

    line();

    while ((row = mysql_fetch_row(result)))
    {
        printf("%-8s %-25s %-18s %-10s %-12s %-12s\n",
               row[0],
               row[1],
               row[2],
               row[3],
               row[4],
               row[5]);
    }

    mysql_free_result(result);
}


/* =====================================================
   3. SEARCH PRODUCT
   ===================================================== */

void searchProduct()
{
    char name[50];
    char query[500];

    MYSQL_RES *result;
    MYSQL_ROW row;

    title("SEARCH PRODUCT");

    printf(YELLOW " Enter product name: " RESET);
    scanf(" %[^\n]", name);

    sprintf(query,
            "SELECT Product_ID, Product_Name, Category, "
            "Quantity, Unit_Price, Supplier_ID "
            "FROM Products "
            "WHERE Product_Name LIKE '%%%s%%'",
            name);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Search failed.");
        return;
    }

    if (mysql_num_rows(result) == 0)
    {
        errorMessage("No product found.");
        mysql_free_result(result);
        return;
    }

    printf("\n");

    while ((row = mysql_fetch_row(result)))
    {
        printf(GREEN "Product ID   : " RESET "%s\n", row[0]);
        printf(GREEN "Product Name : " RESET "%s\n", row[1]);
        printf(GREEN "Category     : " RESET "%s\n", row[2]);
        printf(GREEN "Quantity     : " RESET "%s\n", row[3]);
        printf(GREEN "Unit Price   : " RESET "%s\n", row[4]);
        printf(GREEN "Supplier ID  : " RESET "%s\n", row[5]);

        printf("\n");
    }

    mysql_free_result(result);
}


/* =====================================================
   4. UPDATE PRODUCT STOCK
   ===================================================== */

void updateStock()
{
    int id;
    int quantity;
    char query[300];

    title("UPDATE PRODUCT STOCK");

    printf(YELLOW " Product ID          : " RESET);
    scanf("%d", &id);

    if (!productExists(id))
    {
        errorMessage("Product does not exist.");
        return;
    }

    printf(YELLOW " New Quantity        : " RESET);
    scanf("%d", &quantity);

    if (quantity < 0)
    {
        errorMessage("Quantity cannot be negative.");
        return;
    }

    sprintf(query,
            "UPDATE Products SET Quantity = %d WHERE Product_ID = %d",
            quantity,
            id);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    successMessage("Stock updated successfully.");
}


/* =====================================================
   5. DELETE PRODUCT
   ===================================================== */

void deleteProduct()
{
    int id;
    char query[300];

    title("DELETE PRODUCT");

    printf(YELLOW " Product ID: " RESET);
    scanf("%d", &id);

    if (!productExists(id))
    {
        errorMessage("Product does not exist.");
        return;
    }

    sprintf(query,
            "DELETE FROM Products WHERE Product_ID = %d",
            id);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    successMessage("Product deleted successfully.");
}


/* =====================================================
   6. ADD SUPPLIER
   ===================================================== */

void addSupplier()
{
    Supplier s;

    char query[600];

    title("ADD NEW SUPPLIER");

    printf(YELLOW " Supplier ID   : " RESET);
    scanf("%d", &s.id);

    if (supplierExists(s.id))
    {
        errorMessage("Supplier ID already exists.");
        return;
    }

    printf(YELLOW " Supplier Name : " RESET);
    scanf(" %[^\n]", s.name);

    printf(YELLOW " Contact       : " RESET);
    scanf(" %[^\n]", s.contact);

    printf(YELLOW " Email         : " RESET);
    scanf(" %[^\n]", s.email);

    sprintf(query,
            "INSERT INTO Suppliers "
            "(Supplier_ID, Supplier_Name, Contact, Email) "
            "VALUES (%d, '%s', '%s', '%s')",
            s.id,
            s.name,
            s.contact,
            s.email);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    successMessage("Supplier added successfully.");
}


/* =====================================================
   7. VIEW SUPPLIERS
   ===================================================== */

void viewSuppliers()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("SUPPLIER LIST");

    if (mysql_query(conn,
                    "SELECT Supplier_ID, Supplier_Name, Contact, Email "
                    "FROM Suppliers"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to retrieve suppliers.");
        return;
    }

    printf(CYAN
           "\n %-10s %-25s %-18s %-30s\n"
           RESET,
           "ID",
           "SUPPLIER",
           "CONTACT",
           "EMAIL");

    line();

    while ((row = mysql_fetch_row(result)))
    {
        printf("%-10s %-25s %-18s %-30s\n",
               row[0],
               row[1],
               row[2],
               row[3]);
    }

    mysql_free_result(result);
}


/* =====================================================
   8. SEARCH SUPPLIER
   ===================================================== */

void searchSupplier()
{
    char name[50];
    char query[400];

    MYSQL_RES *result;
    MYSQL_ROW row;

    title("SEARCH SUPPLIER");

    printf(YELLOW " Enter supplier name: " RESET);
    scanf(" %[^\n]", name);

    sprintf(query,
            "SELECT Supplier_ID, Supplier_Name, Contact, Email "
            "FROM Suppliers "
            "WHERE Supplier_Name LIKE '%%%s%%'",
            name);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Search failed.");
        return;
    }

    if (mysql_num_rows(result) == 0)
    {
        errorMessage("Supplier not found.");
        mysql_free_result(result);
        return;
    }

    while ((row = mysql_fetch_row(result)))
    {
        printf("\n");
        printf(GREEN "Supplier ID : " RESET "%s\n", row[0]);
        printf(GREEN "Name        : " RESET "%s\n", row[1]);
        printf(GREEN "Contact     : " RESET "%s\n", row[2]);
        printf(GREEN "Email       : " RESET "%s\n", row[3]);
    }

    mysql_free_result(result);
}


/* =====================================================
   9. ADD QUOTATION
   ===================================================== */

void addQuotation()
{
    Quotation q;

    char query[500];

    title("ADD QUOTATION");

    printf(YELLOW " Quotation ID : " RESET);
    scanf("%d", &q.quotation_id);

    printf(YELLOW " Supplier ID  : " RESET);
    scanf("%d", &q.supplier_id);

    if (!supplierExists(q.supplier_id))
    {
        errorMessage("Supplier does not exist.");
        return;
    }

    printf(YELLOW " Product ID   : " RESET);
    scanf("%d", &q.product_id);

    if (!productExists(q.product_id))
    {
        errorMessage("Product does not exist.");
        return;
    }

    printf(YELLOW " Quantity     : " RESET);
    scanf("%d", &q.quantity);

    printf(YELLOW " Price        : " RESET);
    scanf("%f", &q.price);

    sprintf(query,
            "INSERT INTO Quotations "
            "(Quotation_ID, Supplier_ID, Product_ID, Quantity, Price) "
            "VALUES (%d, %d, %d, %d, %.2f)",
            q.quotation_id,
            q.supplier_id,
            q.product_id,
            q.quantity,
            q.price);

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    successMessage("Quotation added successfully.");
}


/* =====================================================
   10. VIEW QUOTATIONS
   ===================================================== */

void viewQuotations()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("QUOTATION LIST");

    const char *query =
        "SELECT q.Quotation_ID, "
        "s.Supplier_Name, "
        "p.Product_Name, "
        "q.Quantity, "
        "q.Price "
        "FROM Quotations q "
        "JOIN Suppliers s ON q.Supplier_ID = s.Supplier_ID "
        "JOIN Products p ON q.Product_ID = p.Product_ID";

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to retrieve quotations.");
        return;
    }

    printf(CYAN
           "\n %-12s %-25s %-25s %-10s %-12s\n"
           RESET,
           "QUOTATION",
           "SUPPLIER",
           "PRODUCT",
           "QUANTITY",
           "PRICE");

    line();

    while ((row = mysql_fetch_row(result)))
    {
        printf("%-12s %-25s %-25s %-10s %-12s\n",
               row[0],
               row[1],
               row[2],
               row[3],
               row[4]);
    }

    mysql_free_result(result);
}


/* =====================================================
   11. SORT PRODUCTS BY PRICE
   ===================================================== */

void sortProductsByPrice()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("PRODUCTS SORTED BY PRICE");

    const char *query =
        "SELECT Product_ID, Product_Name, Category, "
        "Quantity, Unit_Price "
        "FROM Products "
        "ORDER BY Unit_Price ASC";

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to sort products.");
        return;
    }

    printf(CYAN
           "\n %-8s %-25s %-18s %-10s %-12s\n"
           RESET,
           "ID",
           "PRODUCT",
           "CATEGORY",
           "QUANTITY",
           "PRICE");

    line();

    while ((row = mysql_fetch_row(result)))
    {
        printf("%-8s %-25s %-18s %-10s %-12s\n",
               row[0],
               row[1],
               row[2],
               row[3],
               row[4]);
    }

    mysql_free_result(result);
}


/* =====================================================
   12. LOW STOCK REPORT
   ===================================================== */

void lowStockReport()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("LOW STOCK REPORT");

    const char *query =
        "SELECT Product_ID, Product_Name, Category, Quantity "
        "FROM Products "
        "WHERE Quantity < 10 "
        "ORDER BY Quantity ASC";

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to generate report.");
        return;
    }

    if (mysql_num_rows(result) == 0)
    {
        successMessage("No low-stock products.");
        mysql_free_result(result);
        return;
    }

    printf(CYAN
           "\n %-8s %-25s %-20s %-10s\n"
           RESET,
           "ID",
           "PRODUCT",
           "CATEGORY",
           "QUANTITY");

    line();

    while ((row = mysql_fetch_row(result)))
    {
        printf(RED
               "%-8s %-25s %-20s %-10s\n"
               RESET,
               row[0],
               row[1],
               row[2],
               row[3]);
    }

    mysql_free_result(result);
}


/* =====================================================
   13. TOTAL INVENTORY VALUE
   ===================================================== */

void inventoryValue()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("TOTAL INVENTORY VALUE");

    const char *query =
        "SELECT SUM(Quantity * Unit_Price) "
        "FROM Products";

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to calculate inventory value.");
        return;
    }

    row = mysql_fetch_row(result);

    printf("\n");
    printf(GREEN
           " Total Inventory Value : Rs. %.2f\n"
           RESET,
           row[0] ? atof(row[0]) : 0.0);

    mysql_free_result(result);
}


/* =====================================================
   14. BEST SUPPLIER / LOWEST QUOTATION
   ===================================================== */

void bestQuotation()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("LOWEST QUOTATION");

    const char *query =
        "SELECT s.Supplier_Name, "
        "p.Product_Name, "
        "q.Quantity, "
        "q.Price "
        "FROM Quotations q "
        "JOIN Suppliers s ON q.Supplier_ID = s.Supplier_ID "
        "JOIN Products p ON q.Product_ID = p.Product_ID "
        "ORDER BY q.Price ASC "
        "LIMIT 1";

    if (mysql_query(conn, query))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);

    if (result == NULL)
    {
        errorMessage("Unable to retrieve quotation.");
        return;
    }

    if (mysql_num_rows(result) == 0)
    {
        errorMessage("No quotations available.");
        mysql_free_result(result);
        return;
    }

    row = mysql_fetch_row(result);

    printf("\n");

    printf(GREEN "Supplier : " RESET "%s\n", row[0]);
    printf(GREEN "Product  : " RESET "%s\n", row[1]);
    printf(GREEN "Quantity : " RESET "%s\n", row[2]);
    printf(GREEN "Price    : " RESET "Rs. %s\n", row[3]);

    mysql_free_result(result);
}


/* =====================================================
   15. DASHBOARD
   ===================================================== */

void dashboard()
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    title("SMART PROCUREMENT DASHBOARD");

    if (mysql_query(conn,
                    "SELECT COUNT(*) FROM Products"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    printf("\n");
    printf(BLUE " Total Products       : " RESET "%s\n", row[0]);

    mysql_free_result(result);


    if (mysql_query(conn,
                    "SELECT COUNT(*) FROM Suppliers"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    printf(BLUE " Total Suppliers      : " RESET "%s\n", row[0]);

    mysql_free_result(result);


    if (mysql_query(conn,
                    "SELECT COUNT(*) FROM Quotations"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    printf(BLUE " Total Quotations     : " RESET "%s\n", row[0]);

    mysql_free_result(result);


    if (mysql_query(conn,
                    "SELECT SUM(Quantity) FROM Products"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    printf(BLUE " Total Stock Units    : " RESET "%s\n",
           row[0] ? row[0] : "0");

    mysql_free_result(result);


    if (mysql_query(conn,
                    "SELECT SUM(Quantity * Unit_Price) FROM Products"))
    {
        errorMessage(mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    printf(BLUE " Inventory Value      : " RESET
           "Rs. %.2f\n",
           row[0] ? atof(row[0]) : 0.0);

    mysql_free_result(result);
}


/* =====================================================
   MAIN MENU
   ===================================================== */

void menu()
{
    int choice;

    while (1)
    {
        printf(CLEAR);

        printf("\n");

        printf(CYAN
               "+----------------------------------------------------------------------------+\n"
               RESET);

        printf(WHITE
               "|                    SMART PROCUREMENT SYSTEM                              |\n"
               RESET);

        printf(CYAN
               "+----------------------------------------------------------------------------+\n"
               RESET);

        printf("\n");

        printf(BLUE   "  1. " RESET "Dashboard\n");
        printf(BLUE   "  2. " RESET "Add Product\n");
        printf(BLUE   "  3. " RESET "View Products\n");
        printf(BLUE   "  4. " RESET "Search Product\n");
        printf(BLUE   "  5. " RESET "Update Stock\n");
        printf(BLUE   "  6. " RESET "Delete Product\n");

        printf("\n");

        printf(GREEN  "  7. " RESET "Add Supplier\n");
        printf(GREEN  "  8. " RESET "View Suppliers\n");
        printf(GREEN  "  9. " RESET "Search Supplier\n");

        printf("\n");

        printf(YELLOW " 10. " RESET "Add Quotation\n");
        printf(YELLOW " 11. " RESET "View Quotations\n");
        printf(YELLOW " 12. " RESET "Lowest Quotation\n");

        printf("\n");

        printf(MAGENTA " 13. " RESET "Sort Products by Price\n");
        printf(MAGENTA " 14. " RESET "Low Stock Report\n");
        printf(MAGENTA " 15. " RESET "Inventory Value\n");

        printf("\n");

        printf(RED    "  0. " RESET "Exit\n");

        printf("\n");

        printf(YELLOW " Enter choice: " RESET);
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                dashboard();
                pauseScreen();
                break;

            case 2:
                addProduct();
                pauseScreen();
                break;

            case 3:
                viewProducts();
                pauseScreen();
                break;

            case 4:
                searchProduct();
                pauseScreen();
                break;

            case 5:
                updateStock();
                pauseScreen();
                break;

            case 6:
                deleteProduct();
                pauseScreen();
                break;

            case 7:
                addSupplier();
                pauseScreen();
                break;

            case 8:
                viewSuppliers();
                pauseScreen();
                break;

            case 9:
                searchSupplier();
                pauseScreen();
                break;

            case 10:
                addQuotation();
                pauseScreen();
                break;

            case 11:
                viewQuotations();
                pauseScreen();
                break;

            case 12:
                bestQuotation();
                pauseScreen();
                break;

            case 13:
                sortProductsByPrice();
                pauseScreen();
                break;

            case 14:
                lowStockReport();
                pauseScreen();
                break;

            case 15:
                inventoryValue();
                pauseScreen();
                break;

            case 0:
                printf(GREEN
                       "\nThank you for using Smart Procurement System.\n"
                       RESET);

                return;

            default:
                errorMessage("Invalid choice.");
                pauseScreen();
        }
    }
}


/* =====================================================
   MAIN
   ===================================================== */

int main()
{
    connectDatabase();

    menu();

    mysql_close(conn);

    return 0;
}
