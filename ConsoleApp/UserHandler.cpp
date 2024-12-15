// userHandler.cpp
#include "DbHandler.h" 
#include "UserHandler.h"
#include <chrono> 
#include <iomanip>
#include <iostream>

namespace UserHandler {

    /**
     * @brief Displays the current balance for the logged-in user.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the currently logged-in user.
     */
    void CurrentUserDisplayBalance(SQLite::Database& db, int user_id) {
        try {
            double balance = DbHandler::GetClientBalance(user_id, db);
            std::cout << "\nYour current balance is: $" << std::fixed << std::setprecision(2) << balance << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error displaying balance: " << e.what() << std::endl;
        }
    }

    /**
     * @brief Adds an amount to the current balance for the logged-in user.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the currently logged-in user.
     */
    void CurrentUserAddToBalance(SQLite::Database& db, int user_id) {
        double amount = 0.0;

        std::cout << "Enter the amount you want to add to your balance: $";
        std::cin >> amount;

        if (DbHandler::AddToClientBalance(user_id, amount, db)) {
            double new_balance = DbHandler::GetClientBalance(user_id, db);
            std::cout << "Your new balance is: $" << std::fixed << std::setprecision(2) << new_balance << std::endl;
        }
    }

    /**
     * @brief Withdraws an amount from the current balance for the logged-in user.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the currently logged-in user.
     */
    void CurrentUserWithdrawFromBalance(SQLite::Database& db, int user_id) {
        double amount = 0.0;

        std::cout << "Enter the amount you want to withdraw from your balance: $";
        std::cin >> amount;

        if (DbHandler::WithdrawFromClientBalance(user_id, amount, db)) {
            double new_balance = DbHandler::GetClientBalance(user_id, db);
            std::cout << "Your new balance is: $" << std::fixed << std::setprecision(2) << new_balance << std::endl;
        }
    }

    /**
     * @brief Changes the service choice for the logged-in user.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the currently logged-in user.
     */
    void CurrentUserChangeServiceChoice(SQLite::Database& db, int user_id) {
        int new_service_choice = 0;

        std::cout << "Enter your new service choice (1 = Brokerage, 2 = Retirement): ";
        std::cin >> new_service_choice;

        if (DbHandler::ChangeClientServiceChoice(user_id, new_service_choice, db)) {
            std::string service_name = (new_service_choice == DbHandler::BROKERAGE) ? "Brokerage" : "Retirement";
            std::cout << "Your service choice has been successfully updated to " << service_name << "." << std::endl;
        }
    }

    /**
     * @brief Displays a 12-month projection of the account balance for the logged-in user.
     *
     * @param db Reference to the SQLite database object.
     * @param user_id The ID of the currently logged-in user.
     */
    void CurrentUserDisplayProjection(SQLite::Database& db, int user_id) {
        try {
            std::cout <<"Current user id" << user_id;
            // Retrieve current balance and service choice
            SQLite::Statement query(db, "SELECT cash_balance, service_choice FROM Clients WHERE client_id = ?;");
            query.bind(1, user_id);

            if (query.executeStep()) {
                double current_balance = query.getColumn("cash_balance").getDouble();
                int service_choice = query.getColumn("service_choice").getInt();

                double monthly_rate = 0.0;
                std::string service_name;

                if (service_choice == DbHandler::BROKERAGE) {
                    // Brokerage service
                    monthly_rate = 0.07; // 7% monthly interest
                    service_name = "Brokerage";
                }
                else if (service_choice == DbHandler::RETIREMENT) {
                    // Retirement service
                    monthly_rate = 0.05; // 5% monthly interest
                    service_name = "Retirement";
                }
                else {
                    std::cout << "Unknown service choice." << std::endl;
                    return;
                }

                std::cout << "\n12-Month Projection for " << service_name << " Service:" << std::endl;
                std::cout << "Month\tProjected Balance" << std::endl;

                double projected_balance = current_balance;

                for (int month = 1; month <= 12; ++month) {
                    projected_balance *= (1.0 + monthly_rate);
                    std::cout << month << "\t$" << std::fixed << std::setprecision(2) << projected_balance << std::endl;
                }
            }
            else {
                std::cout << "No account information found." << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error displaying projection: " << e.what() << std::endl;
        }
    }

} // namespace UserHandler
