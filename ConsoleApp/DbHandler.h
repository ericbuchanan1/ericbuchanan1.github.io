// dbHandler.h
#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <chrono> 
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for external variables
extern std::unordered_map<std::string, int> loginAttempts;
extern const int maxAttempts;

namespace DbHandler {

    // Enum to represent service choices
    enum ServiceChoice {
        BROKERAGE = 1,
        RETIREMENT = 2
    };

    // Struct to represent a user
    struct User {
		int user_id;
        std::string name;
        std::string user_group;
    };

    /**
     * @brief Initializes the SQLite database by creating necessary tables and inserting default data.
     *
     * @param db Reference to the SQLite database object.
     */
    void initializeDatabase(SQLite::Database& db);

    /**
     * @brief Authenticates a user and retrieves their user_id and user_group if successful.
     *
     * @param user The username.
     * @param pass The plain text password.
     * @param db Reference to the SQLite database object.
     * @param user_id Reference to store the user's ID upon successful authentication.
     * @param user_group Reference to store the user's group upon successful authentication.
     * @return true If authentication is successful.
     * @return false If authentication fails or account is locked.
     */
    bool CheckUserPermissionAccess(const std::string& user, const std::string& pass, SQLite::Database& db, int& user_id, std::string& user_group);

    /**
     * @brief Adds a new user to the system.
     *
     * @param username The username of the new user.
     * @param password The plaintext password of the new user.
     * @param user_group The group of the new user (admin/manager/viewer).
     * @param db Reference to the SQLite database object.
     */
    void AddNewUser(const std::string& username, const std::string& password, const std::string& user_group, SQLite::Database& db);

    /**
     * @brief Retrieves the current balance of a client.
     *
     * @param client_id The ID of the client.
     * @param db Reference to the SQLite database object.
     * @return double The client's current cash balance.
     */
    double GetClientBalance(int client_id, SQLite::Database& db);

    /**
     * @brief Adds a specified amount to a client's balance.
     *
     * @param client_id The ID of the client.
     * @param amount The amount to add.
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails.
     */
    bool AddToClientBalance(int client_id, double amount, SQLite::Database& db);

    /**
     * @brief Withdraws a specified amount from a client's balance.
     *
     * @param client_id The ID of the client.
     * @param amount The amount to withdraw.
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails (e.g., insufficient funds).
     */
    bool WithdrawFromClientBalance(int client_id, double amount, SQLite::Database& db);

    /**
     * @brief Changes the service choice for a client.
     *
     * @param client_id The ID of the client.
     * @param new_service_choice The new service choice (1 = Brokerage, 2 = Retirement).
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails.
     */
    bool ChangeClientServiceChoice(int client_id, int new_service_choice, SQLite::Database& db);

    /**
     * @brief Retrieves a paginated list of users.
     *
     * @param page The current page number (1-based index).
     * @param page_size The number of users per page.
     * @param users Reference to a vector to store the retrieved users.
     * @param db Reference to the SQLite database object.
     * @return true If users are retrieved successfully.
     * @return false If no users are found or an error occurs.
     */
    bool ListUsers(int page, int page_size, std::vector<User>& users, SQLite::Database& db);

    /**
     * @brief Applies monthly interest to all eligible clients based on their service choices.
     *
     * This function updates the cash_balance of each client by multiplying it with the corresponding service rate.
     * It also updates the last_update_date to the current date.
     *
     * @param db Reference to the SQLite database object.
     */
    void ApplyMonthlyInterest(SQLite::Database& db);

    /**
     * @brief Changes a client's cash balance to a new specified value.
     *
     * @param client_id The ID of the client.
     * @param new_balance The new cash balance to set.
     * @param db Reference to the SQLite database object.
     */
    void UpdateClientBalance(int client_id, double new_balance, SQLite::Database& db);

    /**
     * @brief Changes a client's service choice through admin privileges.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the user performing the operation.
     * @param user_group The group of the user (e.g., admin, manager).
     */
    void ChangeCustomerChoice(SQLite::Database& db, int user_id, const std::string& user_group);

} // namespace DbHandler

#endif // DBHANDLER_H
