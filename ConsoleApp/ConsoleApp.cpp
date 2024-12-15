// ConsoleApp.cpp
// Eric Buchanan 12/1/2024
#pragma comment(lib, "SQLiteCpp.lib")
#pragma comment(lib, "sqlite3.lib")

#include <iostream>
#include <memory>
#include <regex>
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <unordered_map>

// Include the dbHandler, userHandler, adminHandler, and passUtils headers
#include "AdminHandler.h"
#include "DbHandler.h"
#include "PassUtils.h" 
#include "UserHandler.h"

using namespace std;

// Function Prototypes
void DisplayInfo(const string& user_group);
bool isValidInput(const string& input);

int main() {
	bool exitApplication = false; // Flag to control the main application loop
	int choice = 0;               // Variable to store user menu choice

	string user;   // Username input
	string pass;   // Password input
	int user_id = -1; // Initialize user_id to an invalid value
	string user_group; // To store the authenticated user's group (e.g., admin, manager, viewer)

	cout << "Hello! Welcome to our Investment Company, created by Eric Buchanan" << endl;

	// Declare a unique_ptr for SQLite::Database to manage database connection
	unique_ptr<SQLite::Database> db;

	try {
		// Initialize the SQLite database (creates the database file if it doesn't exist)
		db = make_unique<SQLite::Database>("investment_company.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		// Initialize the database (create tables if they don't exist and insert default data)
		DbHandler::initializeDatabase(*db);

	}
	catch (const std::exception& e) {
		cerr << "Database error: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Main application loop
	while (!exitApplication) {
		cout << "\nEnter your username: ";
		cin >> user;
		cout << "Enter your password: ";
		cin >> pass;

		// Validate user input to prevent invalid or malicious entries
		if (!isValidInput(user) || !isValidInput(pass)) {
			cout << "Invalid input format. Please try again." << endl;
			continue;
		}

		// Authenticate user and retrieve user_id and user_group
		bool authResult = DbHandler::CheckUserPermissionAccess(user, pass, *db, user_id, user_group);
		if (!authResult) {
			cout << "Invalid username or password. Please try again." << endl;
			continue;
		}

		cout << "Login successful! User ID: " << user_id << ", User Group: " << user_group << endl;

		bool logout = false; // Flag to control the user session loop

		// User session loop
		while (!logout) {
			DisplayInfo(user_group); // Display menu options based on user_group
			cout << "Enter your choice: ";
			cin >> choice;

			if (choice == 1) {
				// DISPLAY your current balance
				UserHandler::CurrentUserDisplayBalance(*db, user_id);
			}
			else if (choice == 2) {
				// ADD to your current balance
				UserHandler::CurrentUserAddToBalance(*db, user_id);
			}
			else if (choice == 3) {
				// TAKE money out of your account
				UserHandler::CurrentUserWithdrawFromBalance(*db, user_id);
			}
			else if (choice == 4) {
				// CHANGE your service choice
				UserHandler::CurrentUserChangeServiceChoice(*db, user_id);
			}
			else if (choice == 5) {
				// DISPLAY 12 month projection of account balance
				UserHandler::CurrentUserDisplayProjection(*db, user_id);
			}
			else if (choice == 6) {
				// DISPLAY Admin Menu or Manager-specific functionalities
				if (user_group == "admin" || user_group == "manager") {
					AdminHandler::DisplayAdminMenu(*db, user_id);
				}
				else {
					cout << "You do not have permission to access the Admin Menu." << endl;
				}
			}
			else if (choice == 7) {
				// APPLY MONTHLY INTEREST
				// Only allow if user_group permits (e.g., admin)
				if (user_group == "admin") {
					DbHandler::ApplyMonthlyInterest(*db);
				}
				else {
					cout << "You do not have permission to perform this action." << endl;
				}
			}
			else if (choice == 0) {
				// Logout
				cout << "Logging out..." << endl;
				logout = true;
			}
			else {
				cout << "Invalid choice. Please try again." << endl;
			}
		}
	}

	cout << "Thank you for using our Investment Company!" << endl;
	return 0;
}


/**
 * @brief Validates user input to ensure it meets the required format.
 * Allow only alphanumeric and underscores, max length 20
 * @param input The input string to validate.
 * @return true If input is valid.
 * @return false If input is invalid.
 */
bool isValidInput(const string& input) {
	std::regex pattern("^[a-zA-Z0-9_!@#$%^&*()\\-_=+]{1,20}$");    
	return std::regex_match(input, pattern);
}

/**
 * @brief Displays the main menu options based on the user's group.
 *
 * @param user_group The group of the user (e.g., admin, manager, viewer).
 */
void DisplayInfo(const string& user_group) {
    cout << "\nPlease select an option from the menu below." << endl;
    cout << "1. Display your current balance" << endl;
    cout << "2. Add to your current balance" << endl;
    cout << "3. Take money out of your account" << endl;
    cout << "4. Change your service choice" << endl;
    cout << "5. Display 12 month projection of account balance" << endl;

    if (user_group == "admin" || user_group == "manager") {
        cout << "6. DISPLAY Admin Menu" << endl;
    }

    if (user_group == "admin") {
        cout << "7. APPLY MONTHLY INTEREST" << endl;
    }

    cout << "0. Logout" << endl;
}
