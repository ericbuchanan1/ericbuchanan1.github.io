// userHandler.h
#ifndef USERHANDLER_H
#define USERHANDLER_H
#pragma once
#include <SQLiteCpp/SQLiteCpp.h>

namespace UserHandler {

	/**
	 * @brief Displays the current balance for the logged-in user.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the currently logged-in user.
	 */
	void CurrentUserDisplayBalance(SQLite::Database& db, int user_id);

	/**
	 * @brief Adds an amount to the current balance for the logged-in user.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the currently logged-in user.
	 */
	void CurrentUserAddToBalance(SQLite::Database& db, int user_id);

	/**
	 * @brief Withdraws an amount from the current balance for the logged-in user.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the currently logged-in user.
	 */
	void CurrentUserWithdrawFromBalance(SQLite::Database& db, int user_id);

	/**
	 * @brief Changes the service choice for the logged-in user.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the currently logged-in user.
	 */
	void CurrentUserChangeServiceChoice(SQLite::Database& db, int user_id);

	/**
	 * @brief Displays a 12-month projection of the account balance for the logged-in user.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the currently logged-in user.
	 */
	void CurrentUserDisplayProjection(SQLite::Database& db, int user_id);

} // namespace UserHandler

#endif // USERHANDLER_H
