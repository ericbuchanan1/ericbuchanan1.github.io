// adminHandler.h
#ifndef ADMINHANDLER_H
#define ADMINHANDLER_H
#pragma once
#include <SQLiteCpp/SQLiteCpp.h>

namespace AdminHandler {

	/**
	 * @brief Displays the admin menu and handles admin-specific functionalities.
	 *
	 * @param db Reference to the SQLite database object.
	 * @param user_id The ID of the admin user performing the operations.
	 */
	void DisplayAdminMenu(SQLite::Database& db, int user_id);

} // namespace AdminHandler

#endif // ADMINHANDLER_H
