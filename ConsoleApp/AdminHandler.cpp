// adminHandler.cpp
#include "AdminHandler.h"
#include "DbHandler.h"
#include "PassUtils.h" 
#include <chrono> 
#include <iomanip>
#include <iostream>
#include <vector>
#pragma once
namespace AdminHandler {

    /**
     * @brief Adds a new client to the system.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the admin user performing the operation.
     */
    void AddNewClient(SQLite::Database& db, int user_id) {
        std::string client_name;
        double initial_balance;
        int service_choice;
        int assigned_user_id;

        std::cout << "Enter new client's name: ";
        std::cin.ignore(); 
        std::getline(std::cin, client_name);
        std::cout << "Enter initial balance: ";
        std::cin >> initial_balance;
        std::cout << "Select service type (1 = Brokerage, 2 = Retirement): ";
        std::cin >> service_choice;

        // Validate service_choice
        if (service_choice != DbHandler::BROKERAGE && service_choice != DbHandler::RETIREMENT) {
            std::cout << "Invalid service type." << std::endl;
            return;
        }

        // Assign the client to the admin user
        assigned_user_id = user_id;

        try {
            // Insert the new client into the Clients table
            SQLite::Statement insertClient(db, "INSERT INTO Clients (name, service_choice, cash_balance, last_update_date, user_id) VALUES (?, ?, ?, ?, ?);");
            insertClient.bind(1, client_name);
            insertClient.bind(2, service_choice);
            insertClient.bind(3, initial_balance);

            // Get current date in YYYY-MM-DD format
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm{};
            localtime_s(&now_tm, &now_time);

            char date_buffer[11];
            std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
            std::string current_date(date_buffer);

            insertClient.bind(4, current_date);
            insertClient.bind(5, assigned_user_id);
            insertClient.exec();

            std::cout << "New client '" << client_name << "' added successfully with an initial balance of $"
                << std::fixed << std::setprecision(2) << initial_balance << "." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding new client: " << e.what() << std::endl;
        }
    }

    /**
     * @brief Changes a client's service choice.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the admin user performing the operation.
     */
    void ChangeClientChoice(SQLite::Database& db, int user_id) {
        DbHandler::ChangeCustomerChoice(db, user_id, "admin");
    }

    /**
     * @brief Adds a new user to the system.
     *
     * @param db Reference to the SQLite database object.
     */
    void AddNewUser(SQLite::Database& db) {
        std::string new_username, new_user_group;

        std::cout << "Enter new user's name: ";
        std::cin.ignore(); 
        std::getline(std::cin, new_username);
        std::cout << "Enter new user's group (admin/manager/viewer): ";
        std::cin >> new_user_group;

        // Validate new_user_group
        if (new_user_group != "admin" && new_user_group != "manager" && new_user_group != "viewer") {
            std::cout << "Invalid user group. Must be 'admin', 'manager', or 'viewer'." << std::endl;
            return;
        }

        // Generate a random password for the new user
        std::string new_password = PassUtils::generateRandomPassword(); 

        // Add the new user
        DbHandler::AddNewUser(new_username, new_password, new_user_group, db);

        // Display the generated password to the admin
        std::cout << "New user added successfully." << std::endl;
        std::cout << "Generated password for '" << new_username << "': " << new_password << std::endl;
        std::cout << "Please ensure to communicate this password securely to the user." << std::endl;
    }

    /**
     * @brief Changes a client's balance.
     *
     * @param db Reference to the SQLite database object.
     */
    void ChangeClientBalance(SQLite::Database& db) {
        int client_id;
        double new_balance;

        std::cout << "Enter client ID: ";
        std::cin >> client_id;
        std::cout << "Enter new balance: ";
        std::cin >> new_balance;

        DbHandler::UpdateClientBalance(client_id, new_balance, db);
    }

    /**
     * @brief Lists users in a paginated manner.
     *
     * @param db Reference to the SQLite database object.
     */
    void ListUsersPaginated(SQLite::Database& db) {
        int page = 1;
        const int page_size = 7;
        bool continue_listing = true;
        int list_choice;
        while (continue_listing) {
            std::vector<DbHandler::User> users;
            bool has_users = DbHandler::ListUsers(page, page_size, users, db);

            if (!has_users) {
                if (page == 1) {
                    std::cout << "No users to display." << std::endl;
                }
                else {
                    std::cout << "No more users to display." << std::endl;
                }
                break;
            }

            std::cout << "\n--- User List (Page " << page << ") ---" << std::endl;
            std::cout << "ID\tName\t\tGroup" << std::endl;
            for (const auto& user : users) {
                std::cout << user.user_id << "\t" << user.name << "\t\t" << user.user_group << std::endl;
            }

            // Pagination options
            std::cout << "\nOptions:" << std::endl;
            std::cout << "1. Next Page" << std::endl;
            if (page > 1) {
                std::cout << "2. Previous Page" << std::endl;
            }
            std::cout << "0. Exit Listing" << std::endl;
            std::cout << "Enter your choice: ";

            std::cin >> list_choice;

            if (list_choice == 1) {
                page++;
            }
            else if (list_choice == 2 && page > 1) {
                page--;
            }
            else if (list_choice == 0) {
                continue_listing = false;
            }
            else {
                std::cout << "Invalid choice. Exiting listing." << std::endl;
                continue_listing = false;
            }
        }

        if (list_choice > 0 && list_choice < 10 ){
	        // After listing, allow admin to select a user to perform operations
	        std::cout << "\nWould you like to perform operations on a user? (y/n): ";
	        char proceed;
	        std::cin >> proceed;
	
	        if (proceed == 'y' || proceed == 'Y') {
	            int selected_user_id;
	            std::cout << "Enter the User ID to perform operations on: ";
	            std::cin >> selected_user_id;
	
	            // Verify if the user exists
	            SQLite::Statement verifyQuery(db, "SELECT user_group FROM Users WHERE user_id = ?;");
	            verifyQuery.bind(1, selected_user_id);
	            if (!verifyQuery.executeStep()) {
	                std::cout << "User ID " << selected_user_id << " does not exist." << std::endl;
	                return;
	            }
	
	            std::string selected_user_group = verifyQuery.getColumn("user_group").getString();
	
	            // Display operations menu for the selected user
	            bool back_to_admin_menu = false;
	            while (!back_to_admin_menu) {
	                std::cout << "\nOperations for User ID " << selected_user_id << ":" << std::endl;
	                std::cout << "1. Add to Balance" << std::endl;
	                std::cout << "2. Withdraw from Balance" << std::endl;
	                std::cout << "3. Change Service Choice" << std::endl;
	                std::cout << "0. Back to Admin Menu" << std::endl;
	                std::cout << "Enter your choice: ";
	                int operation_choice;
	                std::cin >> operation_choice;
	
	                if (operation_choice == 1) {
	                    // Add to Balance
	                    double amount;
	                    std::cout << "Enter the amount to add: $";
	                    std::cin >> amount;
	                    if (DbHandler::AddToClientBalance(selected_user_id, amount, db)) {
	                        double new_balance = DbHandler::GetClientBalance(selected_user_id, db);
	                        std::cout << "New balance for User ID " << selected_user_id << ": $" << std::fixed << std::setprecision(2) << new_balance << std::endl;
	                    }
	                }
	                else if (operation_choice == 2) {
	                    // Withdraw from Balance
	                    double amount;
	                    std::cout << "Enter the amount to withdraw: $";
	                    std::cin >> amount;
	                    if (DbHandler::WithdrawFromClientBalance(selected_user_id, amount, db)) {
	                        double new_balance = DbHandler::GetClientBalance(selected_user_id, db);
	                        std::cout << "New balance for User ID " << selected_user_id << ": $" << std::fixed << std::setprecision(2) << new_balance << std::endl;
	                    }
	                }
	                else if (operation_choice == 3) {
	                    // Change Service Choice
	                    int new_service_choice;
	                    std::cout << "Enter the new service choice (1 = Brokerage, 2 = Retirement): ";
	                    std::cin >> new_service_choice;
	                    if (DbHandler::ChangeClientServiceChoice(selected_user_id, new_service_choice, db)) {
	                        std::string service_name = (new_service_choice == DbHandler::BROKERAGE) ? "Brokerage" : "Retirement";
	                        std::cout << "Service choice updated to " << service_name << " for User ID " << selected_user_id << "." << std::endl;
	                    }
	                }
	                else if (operation_choice == 0) {
	                    // Back to Admin Menu
	                    back_to_admin_menu = true;
	                }
	                else {
	                    std::cout << "Invalid choice. Please try again." << std::endl;
	                }
	            }
        }
        }
    }

    /**
     * @brief Displays the admin menu and delegates options to respective functions.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the admin user performing the operations.
     */
    void DisplayAdminMenu(SQLite::Database& db, int user_id) {
        while (true) {
            std::cout << "\nAdmin Menu:" << std::endl;
            std::cout << "1. Add a new client" << std::endl;
            std::cout << "2. Change a client's choice" << std::endl;
            std::cout << "3. Add a new user to the system" << std::endl;
            std::cout << "4. Change a balance for a client" << std::endl;
            std::cout << "5. List Users (Paginated)" << std::endl;
            std::cout << "0. Back to Main Menu" << std::endl;
            std::cout << "Enter your choice: ";
            int admin_choice;
            std::cin >> admin_choice;

            switch (admin_choice) {
            case 1:
                AddNewClient(db, user_id);
                break;
            case 2:
                ChangeClientChoice(db, user_id);
                break;
            case 3:
                AddNewUser(db);
                break;
            case 4:
                ChangeClientBalance(db);
                break;
            case 5:
                ListUsersPaginated(db);
                break;
            case 0:
                // Back to Main Menu
                return;
            default:
                std::cout << "Invalid admin choice. Please try again." << std::endl;
                break;
            }
        }
    }

} // namespace AdminHandler
