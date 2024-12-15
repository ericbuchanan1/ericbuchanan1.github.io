#pragma once
// dbHandler.cpp
#include "DbHandler.h"
#include "PassUtils.h"
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <SQLiteCpp/SQLiteCpp.h>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace std {
    template <>
    struct hash<DbHandler::ServiceChoice> {
        std::size_t operator()(const DbHandler::ServiceChoice& sc) const noexcept {
            return std::hash<int>()(static_cast<int>(sc));
        }
    };
}

namespace DbHandler {

    // Define a mapping from ServiceChoice to monthly interest rates
    static const std::unordered_map<ServiceChoice, double> serviceRates = {
        {BROKERAGE, 1.07},    // 7% monthly interest rate
        {RETIREMENT, 1.05}    // 5% monthly interest rate
    };

    // External variables declared elsewhere (e.g., in main.cpp)
    // Defined in dbHandler.h as extern, need to define here
    std::unordered_map<std::string, int> loginAttempts;
    const int maxAttempts = 3;

    /**
     * @brief Initializes the SQLite database by creating necessary tables and inserting default data.
     *
     * @param db Reference to the SQLite database object.
     */
	void initializeDatabase(SQLite::Database& db) {
		try {
			// Create the Users table
			db.exec("CREATE TABLE IF NOT EXISTS Users ("
				"user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
				"name TEXT NOT NULL UNIQUE,"
				"hashed_password TEXT NOT NULL,"
				"salt TEXT NOT NULL,"
				"user_group TEXT NOT NULL"
				");");

			// Create the Clients table with last_update_date
			db.exec("CREATE TABLE IF NOT EXISTS Clients ("
				"client_id INTEGER PRIMARY KEY AUTOINCREMENT,"
				"name TEXT NOT NULL UNIQUE,"
				"service_choice INTEGER NOT NULL,"
				"cash_balance REAL NOT NULL DEFAULT 0.0,"
				"last_update_date TEXT,"
				"user_id INTEGER NOT NULL,"
				"FOREIGN KEY(user_id) REFERENCES Users(user_id) ON DELETE CASCADE"
				");");

			// Insert default admin user if not exists
			SQLite::Statement query(db, "SELECT COUNT(*) as count FROM Users WHERE name = ?;");
			query.bind(1, "admin");
			if (query.executeStep() && query.getColumn("count").getInt() == 0) {
				std::string salt = PassUtils::generateSalt();
				std::string hashed_password = PassUtils::hashPasswordPBKDF2("admin", salt);
				SQLite::Statement insert(db, "INSERT INTO Users (name, hashed_password, salt, user_group) VALUES (?, ?, ?, ?);");
				insert.bind(1, "admin");
				insert.bind(2, hashed_password);
				insert.bind(3, salt);
				insert.bind(4, "admin");
				insert.exec();
				std::cout << "Default admin user created." << std::endl;
			}

			// Insert default clients if Clients table is empty
			SQLite::Statement clientCheckQuery(db, "SELECT COUNT(*) as count FROM Clients;");
			if (clientCheckQuery.executeStep() && clientCheckQuery.getColumn("count").getInt() == 0) {
				std::vector<std::pair<std::string, ServiceChoice>> defaultClients = {
					{"Bob Jones", BROKERAGE},
					{"Sarah Davis", RETIREMENT},
					{"Amy Friendly", BROKERAGE},
					{"Jonny Smith", BROKERAGE},
					{"Carol Spears", RETIREMENT}
				};

				auto now = std::chrono::system_clock::now();
				std::time_t now_time = std::chrono::system_clock::to_time_t(now);
				std::tm now_tm{};
				localtime_s(&now_tm, &now_time);
				char date_buffer[11];
				std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
				std::string current_date(date_buffer);

				for (const auto& client : defaultClients) {
					// Extract last name and generate username
					size_t space_pos = client.first.find(' ');
					std::string last_name = client.first.substr(space_pos + 1);
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_int_distribution<> dist(100, 999);
					std::string username = last_name + std::to_string(dist(gen));
					std::cout << "Generated username for " << client.first << ": " << username << std::endl;

					// Create user account for the client
					std::string password = "password1";
					std::string salt = PassUtils::generateSalt();
					std::string hashed_password = PassUtils::hashPasswordPBKDF2(password, salt);
					SQLite::Statement insertUser(db, "INSERT INTO Users (name, hashed_password, salt, user_group) VALUES (?, ?, ?, ?);");
					insertUser.bind(1, username);
					insertUser.bind(2, hashed_password);
					insertUser.bind(3, salt);
					insertUser.bind(4, "user");
					insertUser.exec();

					// Insert client linked to user account
					SQLite::Statement insertClient(db, "INSERT INTO Clients (name, service_choice, cash_balance, last_update_date, user_id) VALUES (?, ?, ?, ?, ?);");
					insertClient.bind(1, client.first);
					insertClient.bind(2, static_cast<int>(client.second));
					insertClient.bind(3, 0.0);
					insertClient.bind(4, current_date);
					insertClient.bind(5, db.getLastInsertRowid());
					insertClient.exec();
				}
				std::cout << "Default clients and linked user accounts added to the database." << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Database initialization error: " << e.what() << std::endl;
			exit(EXIT_FAILURE);
		}
	}


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
    bool CheckUserPermissionAccess(const std::string& user, const std::string& pass, SQLite::Database& db, int& user_id, std::string& user_group) {
        // Check if the account is locked due to too many failed login attempts
        if (loginAttempts[user] >= maxAttempts) {
            std::cout << "Account locked due to too many failed attempts." << std::endl;
            return false;
        }

        try {
            // Prepare and execute the query to fetch user details
            SQLite::Statement query(db, "SELECT user_id, hashed_password, salt, user_group FROM Users WHERE name = ?;");
            query.bind(1, user);

            if (query.executeStep()) {
                // Retrieve user details
                user_id = query.getColumn("user_id").getInt();
                std::string stored_hash = query.getColumn("hashed_password").getString();
                std::string stored_salt = query.getColumn("salt").getString();
                user_group = query.getColumn("user_group").getString();

                // Verify the provided password against the stored hash
                if (PassUtils::verifyPasswordPBKDF2(pass, stored_salt, stored_hash)) {
                    loginAttempts[user] = 0; // Reset login attempts on successful login
                    return true;
                }
            }

            // If authentication fails, increment the login attempt counter
            loginAttempts[user]++;
            if (loginAttempts[user] >= maxAttempts) {
                std::cout << "Account locked due to too many failed attempts." << std::endl;
            }

            return false;

        }
        catch (const std::exception& e) {
            std::cerr << "Authentication error: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief Adds a new user to the system.
     *
     * @param username The username of the new user.
     * @param password The plaintext password of the new user.
     * @param user_group The group of the new user (admin/manager/viewer).
     * @param db Reference to the SQLite database object.
     */
    void AddNewUser(const std::string& username, const std::string& password, const std::string& user_group, SQLite::Database& db) {
        try {
            // Check if the username already exists
            SQLite::Statement checkQuery(db, "SELECT COUNT(*) as count FROM Users WHERE name = ?;");
            checkQuery.bind(1, username);
            if (checkQuery.executeStep() && checkQuery.getColumn("count").getInt() > 0) {
                std::cout << "Username already exists. Please choose a different name." << std::endl;
                return;
            }

            // Generate a random salt for password hashing
            std::string salt = PassUtils::generateSalt();
            // Hash the password with the generated salt
            std::string hashed_password = PassUtils::hashPasswordPBKDF2(password, salt);

            // Insert the new user into the Users table
            SQLite::Statement insertUser(db, "INSERT INTO Users (name, hashed_password, salt, user_group) VALUES (?, ?, ?, ?);");
            insertUser.bind(1, username);
            insertUser.bind(2, hashed_password);
            insertUser.bind(3, salt);
            insertUser.bind(4, user_group);
            insertUser.exec();

            std::cout << "New user '" << username << "' added successfully as '" << user_group << "'." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding new user: " << e.what() << std::endl;
        }
    }

    /**
     * @brief Retrieves the current balance of a client.
     *
     * @param client_id The ID of the client.
     * @param db Reference to the SQLite database object.
     * @return double The client's current cash balance.
     */
	double GetClientBalance(int client_id, SQLite::Database& db) {
		try {
			SQLite::Statement query(db, "SELECT cash_balance FROM Clients WHERE client_id = ?;");
			query.bind(1, client_id);
			if (query.executeStep()) {
				return query.getColumn("cash_balance").getDouble();
			}
			else {
				std::cerr << "Client ID " << client_id << " not found." << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error retrieving client balance: " << e.what() << std::endl;
		}
		return 0.0;
	}

    /**
     * @brief Adds a specified amount to a client's balance.
     *
     * @param client_id The ID of the client.
     * @param amount The amount to add.
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails.
     */
	bool AddToClientBalance(int client_id, double amount, SQLite::Database& db) {
		if (amount <= 0) {
			std::cout << "Amount must be positive." << std::endl;
			return false;
		}

		// Ensure the amount has only two decimal places
		amount = std::round(amount * 100.0) / 100.0;

		try {
			// Get the current balance
			double current_balance = 0.0;
			SQLite::Statement getBalance(db, "SELECT cash_balance FROM Clients WHERE client_id = ?;");
			getBalance.bind(1, client_id);
			if (getBalance.executeStep()) {
				current_balance = getBalance.getColumn("cash_balance").getDouble();
			}
			else {
				std::cout << "Client ID " << client_id << " not found." << std::endl;
				return false;
			}

			// Update the balance
			SQLite::Statement update(db, "UPDATE Clients SET cash_balance = cash_balance + ?, last_update_date = ? WHERE client_id = ?;");
			update.bind(1, amount);

			auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm;
			localtime_s(&now_tm, &now_time);
			char date_buffer[11];
			std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
			std::string current_date(date_buffer);

			update.bind(2, current_date);
			update.bind(3, client_id);
			update.exec();

			double updated_balance = current_balance + amount;
			std::cout << "Successfully added $" << std::fixed << std::setprecision(2) << amount
				<< " to client ID " << client_id << "'s account. Current balance: $"
				<< updated_balance << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error adding to client balance: " << e.what() << std::endl;
			return false;
		}
	}

    /**
     * @brief Withdraws a specified amount from a client's balance.
     *
     * @param client_id The ID of the client.
     * @param amount The amount to withdraw.
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails (e.g., insufficient funds).
     */
	bool WithdrawFromClientBalance(int client_id, double amount, SQLite::Database& db) {
		if (amount <= 0) {
			std::cout << "Amount must be positive." << std::endl;
			return false;
		}

		// Ensure the amount has only two decimal places
		amount = std::round(amount * 100.0) / 100.0;

		try {
			// Get the current balance
			double current_balance = 0.0;
			SQLite::Statement getBalance(db, "SELECT cash_balance FROM Clients WHERE client_id = ?;");
			getBalance.bind(1, client_id);
			if (getBalance.executeStep()) {
				current_balance = getBalance.getColumn("cash_balance").getDouble();
			}
			else {
				std::cout << "Client ID " << client_id << " not found." << std::endl;
				return false;
			}

			if (amount > current_balance) {
				std::cout << "Insufficient funds. Current balance: $" << std::fixed << std::setprecision(2) << current_balance << std::endl;
				return false;
			}

			// Update the balance
			SQLite::Statement update(db, "UPDATE Clients SET cash_balance = cash_balance - ?, last_update_date = ? WHERE client_id = ?;");
			update.bind(1, amount);

			auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm{};
			localtime_s(&now_tm, &now_time);
			char date_buffer[11];
			std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
			std::string current_date(date_buffer);

			update.bind(2, current_date);
			update.bind(3, client_id);
			update.exec();

			double updated_balance = current_balance - amount;
			std::cout << "Successfully withdrew $" << std::fixed << std::setprecision(2) << amount
				<< " from client ID " << client_id << ". Current balance: $"
				<< updated_balance << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Error withdrawing from client balance: " << e.what() << std::endl;
			return false;
		}
	}


    /**
     * @brief Changes the service choice for a client.
     *
     * @param client_id The ID of the client.
     * @param new_service_choice The new service choice (1 = Brokerage, 2 = Retirement).
     * @param db Reference to the SQLite database object.
     * @return true If the operation is successful.
     * @return false If the operation fails.
     */
    bool ChangeClientServiceChoice(int client_id, int new_service_choice, SQLite::Database& db) {
        if (new_service_choice != BROKERAGE && new_service_choice != RETIREMENT) {
            std::cout << "Invalid service choice. Must be 1 (Brokerage) or 2 (Retirement)." << std::endl;
            return false;
        }

        try {
            // Update the client's service choice
            SQLite::Statement update(db, "UPDATE Clients SET service_choice = ?, last_update_date = ? WHERE client_id = ?;");
            update.bind(1, new_service_choice);

            // Get current date in YYYY-MM-DD format
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm;
            localtime_s(&now_tm, &now_time);

            char date_buffer[11];
            std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
            std::string current_date(date_buffer);

            update.bind(2, current_date);
            update.bind(3, client_id);
            update.exec();

            std::string service_name = (new_service_choice == BROKERAGE) ? "Brokerage" : "Retirement";
            std::cout << "Client ID " << client_id << "'s service choice has been updated to " << service_name << "." << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error changing client service choice: " << e.what() << std::endl;
            return false;
        }
    }

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
    bool ListUsers(int page, int page_size, std::vector<User>& users, SQLite::Database& db) {
        try {
            int offset = (page - 1) * page_size;
            SQLite::Statement query(db, "SELECT user_id, name, user_group FROM Users ORDER BY user_id LIMIT ? OFFSET ?;");
            query.bind(1, page_size);
            query.bind(2, offset);

            while (query.executeStep()) {
                User user;
                user.user_id = query.getColumn("user_id").getInt();
                user.name = query.getColumn("name").getString();
                user.user_group = query.getColumn("user_group").getString();
                users.push_back(user);
            }

            if (users.empty()) {
                return false;
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error listing users: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief Applies monthly interest to all eligible clients based on their service choices.
     * @note Skips monthly check, is manual whenever this button is hit.
     *
     * This function updates the cash_balance of each client by multiplying it with the corresponding service rate.
     * It also updates the last_update_date to the current date.
     *
     * @param db Reference to the SQLite database object.
     */
	void ApplyMonthlyInterest(SQLite::Database& db) {
        const bool OVERIDE_MONTHLY_TIMER = true;
        
        
        try {
			// Get the current date in YYYY-MM-DD format
			auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);
			std::tm now_tm{};
			localtime_s(&now_tm, &now_time);

			char date_buffer[11]; // YYYY-MM-DD
			std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
			std::string current_date(date_buffer);

			// Query all clients
			SQLite::Statement query(db, "SELECT client_id, cash_balance, service_choice, last_update_date FROM Clients;");
			while (query.executeStep()) {
				int client_id = query.getColumn("client_id").getInt();
				double balance = query.getColumn("cash_balance").getDouble();
				int service_choice_int = query.getColumn("service_choice").getInt();
				std::string last_update_date = query.getColumn("last_update_date").isNull() ? "" : query.getColumn("last_update_date").getString();

				// Get the service choice as an enum
				ServiceChoice choice = static_cast<ServiceChoice>(service_choice_int);

				// Find the interest rate for the service choice
				auto rate_it = serviceRates.find(choice);
				if (rate_it == serviceRates.end()) {
					std::cerr << "Invalid service choice for client ID " << client_id << std::endl;
					continue;
				}
				double monthly_rate = rate_it->second;

				// Check if a month has passed since the last update
				bool apply_interest = false;
				if (last_update_date.empty()) {
					// If no last update date, apply interest
					apply_interest = true;
				}
				else {
					// Parse last_update_date
					std::tm last_tm{};
					std::istringstream ss(last_update_date);
					ss >> std::get_time(&last_tm, "%Y-%m-%d");
					if (ss.fail()) {
						std::cerr << "Failed to parse last_update_date for client ID " << client_id << std::endl;
						continue;
					}

					// Check if at least a month has passed
					int months_diff = (now_tm.tm_year - last_tm.tm_year) * 12 + (now_tm.tm_mon - last_tm.tm_mon);
					if (months_diff >= 1) {
						apply_interest = true;
					}
				}
                if (OVERIDE_MONTHLY_TIMER) {                        // This is the override setting 
                    apply_interest = true;
                }

				if (apply_interest) {
					// Apply interest to the balance
					double new_balance = balance * monthly_rate;

					// Update the balance and last update date in the database
					SQLite::Statement update(db, "UPDATE Clients SET cash_balance = ?, last_update_date = ? WHERE client_id = ?;");
					update.bind(1, new_balance);
					update.bind(2, current_date);
					update.bind(3, client_id);
					update.exec();

					std::cout << "Updated balance for client ID " << client_id << " from $" << balance
						<< " to $" << new_balance << " using rate " << (monthly_rate - 1) * 100 << "%." << std::endl;
				}
			}

			std::cout << "Monthly interest applied to eligible clients." << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Error applying monthly interest: " << e.what() << std::endl;
		}
	}


    /**
     * @brief Changes a client's service choice through admin privileges.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the user performing the operation.
     * @param user_group The group of the user (e.g., admin, manager).
     */
    void ChangeCustomerChoice(SQLite::Database& db, int user_id, const std::string& user_group) {
        int client_id = -1;
        int update = -1;

        while (true) {
            // Display the list of clients the user has access to
            std::cout << "\nEnter the client ID you wish to change (or 0 to Exit):\n";

            // Prepare the query based on user_group
            SQLite::Statement listQuery(db, "SELECT client_id, name, service_choice FROM Clients WHERE user_id = ? OR ? = 'admin';");
            listQuery.bind(1, user_id);
            listQuery.bind(2, user_group);

            bool has_clients = false;
            while (listQuery.executeStep()) {
                int cid = listQuery.getColumn("client_id").getInt();
                std::string cname = listQuery.getColumn("name").getString();
                int service_choice = listQuery.getColumn("service_choice").getInt();
                std::string service_name = (service_choice == BROKERAGE) ? "Brokerage" : "Retirement";
                std::cout << cid << ". " << cname << " (Service: " << service_name << ")" << std::endl;
                has_clients = true;
            }

            if (!has_clients) {
                std::cout << "No clients available to modify." << std::endl;
                return;
            }

            std::cout << "0. Exit" << std::endl;
            std::cout << "Enter your choice: ";
            std::cin >> client_id;

            if (client_id == 0) {
                break;
            }

            // Validate that the chosen client exists and the user has permission
            SQLite::Statement validateQuery(db, "SELECT service_choice FROM Clients WHERE client_id = ? AND (user_id = ? OR ? = 'admin');");
            validateQuery.bind(1, client_id);
            validateQuery.bind(2, user_id);
            validateQuery.bind(3, user_group);

            if (!validateQuery.executeStep()) {
                std::cout << "Invalid client ID or insufficient permissions. Please try again." << std::endl;
                continue;
            }

            // Prompt for new service choice
            while (true) {
                std::cout << "Please enter the client's new service choice:\n";
                std::cout << "1 = Brokerage\n2 = Retirement\n0 = Cancel\n";
                std::cout << "Enter your choice: ";
                std::cin >> update;

                if (update == 0) {
                    std::cout << "Update canceled." << std::endl;
                    break;
                }
                if (update != BROKERAGE && update != RETIREMENT) {
                    std::cout << "Invalid service choice. Please try again." << std::endl;
                }
                else {
                    // Update the service_choice in the database
                    try {
                        if (ChangeClientServiceChoice(client_id, update, db)) {
                            std::string service_name = (update == BROKERAGE) ? "Brokerage" : "Retirement";
                            std::cout << "The client's service choice has been updated to " << service_name << "." << std::endl;
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Failed to update client in database: " << e.what() << std::endl;
                    }

                    break;
                }
            }
        }
    }

    /**
     * @brief Changes a client's cash balance.
     *
     * @param client_id The ID of the client.
     * @param new_balance The new cash balance to set.
     * @param db Reference to the SQLite database object.
     */
    void UpdateClientBalance(int client_id, double new_balance, SQLite::Database& db) {
        try {
            // Get current date in YYYY-MM-DD format
            auto now = std::chrono::system_clock::now();
            std::time_t now_time = std::chrono::system_clock::to_time_t(now);

            std::tm now_tm{};
            localtime_s(&now_tm, &now_time);

            char date_buffer[11]; //YYYY-MM-DD
            std::strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", &now_tm);
            std::string current_date(date_buffer);

            // Update the client's cash balance and last_update_date
            SQLite::Statement update(db, "UPDATE Clients SET cash_balance = ?, last_update_date = ? WHERE client_id = ?;");
            update.bind(1, new_balance);
            update.bind(2, current_date);
            update.bind(3, client_id);
            update.exec();

            std::cout << "Client ID " << client_id << " balance updated to $" << new_balance << "." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error updating client balance: " << e.what() << std::endl;
        }
    }

} // namespace DbHandler