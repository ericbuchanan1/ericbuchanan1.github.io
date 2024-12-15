package com.example.androidapp.dbHelper;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;
import android.util.Pair;
import com.example.androidapp.utils.passUtils;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

/**
 * dbConnector is a helper class that manages database creation, version management,
 * and provides methods to perform CRUD operations on the application's SQLite database.
 */
public class dbConnector extends SQLiteOpenHelper {
    private static final String DATABASE_NAME = "AppDatabase.db";
    private static final int DATABASE_VERSION = 2;

    /**
     * Constructor for dbConnector.
     *
     * @param context The context of the activity or application.
     */
    public dbConnector(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    /**
     * Called when the database is created for the first time.
     *
     * @param db The database.
     */
    @Override
    public void onCreate(SQLiteDatabase db) {
        // Create 'users' table to store user credentials and roles
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT NOT NULL UNIQUE," +
                        "password TEXT NOT NULL," +
                        "phone TEXT NOT NULL," +
                        "role TEXT NOT NULL);"
        );

        // Create 'user_data' table to store daily weight entries
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS user_data (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "user_id INTEGER NOT NULL," +
                        "date TEXT NOT NULL," +
                        "weight INTEGER NOT NULL CHECK(weight > 0)," +
                        "goal_value INTEGER," +
                        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE);"
        );

        // Create 'goal' table to store user goals
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS goal (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "user_id INTEGER NOT NULL," +
                        "goal_value INTEGER NOT NULL CHECK(goal_value > 0)," +
                        "target_date TEXT," +
                        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE);"
        );

        // Create 'session' table to manage user sessions
        db.execSQL(
                "CREATE TABLE IF NOT EXISTS session (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "user_id INTEGER," +
                        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE SET NULL);"
        );

        // Create indexes to optimize query performance
        db.execSQL("CREATE INDEX idx_user_data_user_id ON user_data(user_id);");
        db.execSQL("CREATE INDEX idx_goal_user_id ON goal(user_id);");
        db.execSQL("CREATE INDEX idx_session_user_id ON session(user_id);");
    }

    /**
     * Called when the database needs to be upgraded.
     *
     * @param db         The database.
     * @param oldVersion The old database version.
     * @param newVersion The new database version.
     */
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // Drop existing tables to recreate them
        db.execSQL("DROP TABLE IF EXISTS users");
        db.execSQL("DROP TABLE IF EXISTS user_data");
        db.execSQL("DROP TABLE IF EXISTS goal");
        db.execSQL("DROP TABLE IF EXISTS session");
        onCreate(db);
    }

    /**
     * Registers a new user in the database.
     *
     * @param user         The username.
     * @param pass         The plaintext password.
     * @param phone_number The user's phone number.
     * @param role         The role code ("537" for admin, "1237" for paid, others for user).
     * @return 1 if registration is successful,
     * -1 if the username is taken,
     * -3 for database errors.
     */
    public int registerUser(String user, String pass, String phone_number, String role) {
        // Validate input parameters
        if (user == null || pass == null || phone_number == null || role == null) {
            Log.e("RegisterError", "Invalid input parameters.");
            return -3; // Database error
        }

        SQLiteDatabase db = null;
        try {
            db = this.getWritableDatabase();
            db.beginTransaction(); // Begin transaction for atomicity

            // Check if the username already exists
            try (Cursor cursor = db.rawQuery("SELECT username FROM users WHERE username = ?", new String[]{user})) {
                if (cursor.moveToFirst()) {
                    Log.w("RegisterWarning", "Username already taken: " + user);
                    return -1; // Username taken
                }
            }

            // Map role codes to role strings
            switch (role) {
                case "537":
                    role = "admin";
                    break;
                case "1237":
                    role = "paid";
                    break;
                default:
                    role = "user";
            }

            // Hash the plaintext password for security
            String hashedPassword = passUtils.hashPassword(pass);
            //Log.d("RegisterSuccess", "Hashed Password: " + hashedPassword);
            // NOTE: Only here for debugging reasons

            // Prepare values for insertion
            ContentValues values = new ContentValues();
            values.put("username", user);
            values.put("password", hashedPassword);
            values.put("phone", phone_number);
            values.put("role", role);

            // Insert the new user into the 'users' table
            // Convert the result from long to int safely
            int result = Math.toIntExact(db.insert("users", null, values));
            if (result == -1) {
                Log.e("RegisterError", "Failed to insert new user.");
                return -3; // Database error
            }

            db.setTransactionSuccessful(); // Mark transaction as successful

            setCurrentUserId(result); // Set the current user session
            Log.d("RegisterSuccess", "User registered with ID: " + result);
            return 1; // Successful registration
        } catch (SQLiteException e) {
            Log.e("RegisterException", e.toString());
            return -3; // Database error
        } finally {
            if (db != null) {
                if (db.inTransaction()) {
                    db.endTransaction(); // Ensure the transaction is ended
                }
            }
        }
    }

    /**
     * Logs in a user by verifying their credentials.
     *
     * @param user The username.
     * @param pass The plaintext password.
     * @return 1 if login is successful,
     * -2 if credentials are invalid,
     * -3 for database errors.
     */
    public int loginUser(String user, String pass) {
        // Validate input parameters
        if (user == null || pass == null) {
            Log.e("LoginError", "Username or password is null.");
            return -2; // Invalid credentials
        }

        try {
            SQLiteDatabase db = this.getReadableDatabase();
            // Query to retrieve user ID and hashed password
            try (Cursor cursor = db.rawQuery("SELECT id, password FROM users WHERE username = ?", new String[]{user})) {
                if (cursor.moveToFirst()) {
                    int userIdIndex = cursor.getColumnIndex("id");
                    int passwordIndex = cursor.getColumnIndex("password");

                    if (userIdIndex != -1 && passwordIndex != -1) {
                        int userId = cursor.getInt(userIdIndex);
                        String storedHash = cursor.getString(passwordIndex);
                        //Log.d("LoginDebug", "Stored hashed password: " + storedHash);
                        //Log.d("LoginDebug", "Plaintext password entered: " + pass);
                        // Note: Only here for debugging purposes


                        // Verify the entered password against the stored hash
                        if (passUtils.verifyPassword(pass, storedHash)) {
                            this.setCurrentUserId(userId); // Set the current user session
                            Log.d("LoginSuccess", "User '" + user + "' logged in successfully with ID: " + userId);
                            return 1; // Successful login
                        } else {
                            Log.w("LoginWarning", "Invalid password for user: " + user);
                            return -2; // Invalid credentials
                        }
                    } else {
                        Log.e("LoginError", "Required columns not found in the cursor.");
                        return -1; // Column not found error
                    }
                } else {
                    Log.w("LoginWarning", "User not found: " + user);
                    return -2; // Invalid credentials
                }
            }
        } catch (SQLiteException e) {
            Log.e("LoginException", e.toString());
            return -3; // Database error
        }
    }

    /**
     * Sets the current user ID in the 'session' table.
     *
     * @param userId The ID of the user to set as current.
     */
    private void setCurrentUserId(int userId) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("user_id", userId);
        db.delete("session", null, null); // Clear existing sessions
        db.insert("session", null, values); // Insert the new session
    }

    /**
     * Retrieves the current logged-in user ID from the 'session' table.
     *
     * @return The user ID if a session exists, otherwise -1.
     */
    public int getCurrentUserID() {
        SQLiteDatabase db = this.getReadableDatabase();
        int userId = -1;
        try (Cursor cursor = db.rawQuery("SELECT user_id FROM session LIMIT 1", null)) {
            if (cursor.moveToFirst()) {
                userId = cursor.getInt(cursor.getColumnIndexOrThrow("user_id"));
            }
        } catch (SQLiteException e) {
            Log.e("DBError", "Error fetching current user ID", e);
        } finally {
            Log.d("GetCurrentUserID", "Current user ID: " + userId);
        }
        return userId;
    }

    /**
     * Sets or updates the goal weight for a user.
     *
     * @param userId     The ID of the user.
     * @param goalWeight The goal weight to set.
     * @return True if the operation is successful, false otherwise.
     */
    public boolean setGoalWeight(int userId, int goalWeight) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("user_id", userId);
        values.put("goal_value", goalWeight);

        // Insert with conflict resolution to replace existing entries
        return db.insertWithOnConflict("goal", null, values, SQLiteDatabase.CONFLICT_REPLACE) != -1;
    }

    /**
     * Retrieves the most recent goal weight for a user.
     *
     * @param userId The ID of the user.
     * @return The most recent goal weight if set, otherwise -1.
     */
    public int getMostRecentGoalWeight(int userId) {
        SQLiteDatabase db = this.getReadableDatabase();
        try (Cursor cursor = db.rawQuery("SELECT goal_value FROM goal WHERE user_id = ? ORDER BY id DESC LIMIT 1", new String[]{String.valueOf(userId)})) {
            if (cursor.moveToFirst()) {
                return cursor.getInt(cursor.getColumnIndexOrThrow("goal_value"));
            }
        } catch (SQLiteException e) {
            Log.e("DBError", "Error fetching most recent goal weight", e);
        }
        return -1; // Return -1 if no goal weight is found
    }

    /**
     * Adds a daily weight entry for a user.
     *
     * @param userId The ID of the user.
     * @param weight The weight to add.
     * @return True if the entry is added successfully, false otherwise.
     */
    public boolean addDailyWeight(int userId, int weight) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("user_id", userId);
        values.put("weight", weight);
        String currentDate = new SimpleDateFormat("MM-dd-yyyy", Locale.getDefault()).format(new Date());
        values.put("date", currentDate);
        int currentGoalWeight = getMostRecentGoalWeight(userId);
        if (currentGoalWeight == -1) {
            Log.e("AddWeightError", "No goal weight set for user");
            return false;
        }

        values.put("goal_value", currentGoalWeight);
        return db.insert("user_data", null, values) != -1;
    }

    /**
     * Retrieves the most recent target date for a user's goal.
     *
     * @param userId The ID of the user.
     * @return The most recent target date if set, otherwise null.
     */
    public String getMostRecentTargetDate(int userId) {
        String targetDate = null;
        SQLiteDatabase db = this.getReadableDatabase();

        try (Cursor cursor = db.rawQuery("SELECT target_date FROM goal WHERE user_id = ? ORDER BY id DESC LIMIT 1",
                new String[]{String.valueOf(userId)})) {
            if (cursor.moveToFirst()) {
                targetDate = cursor.getString(cursor.getColumnIndexOrThrow("target_date"));
            }
        } catch (SQLiteException e) {
            Log.e("DB_ERROR", "Error fetching most recent target date: " + e.getMessage());
        }
        return targetDate;
    }

    /**
     * Retrieves all weight entries along with their target weights and dates for a user.
     *
     * @param userId The ID of the user.
     * @return A list of pairs containing weight, goal value, and date.
     */
    public List<Pair<Pair<Integer, Integer>, String>> getUserWeightsWithTarget(int userId) {
        List<Pair<Pair<Integer, Integer>, String>> weights = new ArrayList<>();
        SQLiteDatabase db = this.getReadableDatabase();

        try (Cursor cursor = db.rawQuery(
                "SELECT weight, goal_value, date FROM user_data WHERE user_id = ?", new String[]{String.valueOf(userId)})) {
            while (cursor.moveToNext()) {
                int weight = cursor.getInt(cursor.getColumnIndexOrThrow("weight"));
                int goal_value = cursor.getInt(cursor.getColumnIndexOrThrow("goal_value"));
                String date = cursor.getString(cursor.getColumnIndexOrThrow("date"));
                weights.add(new Pair<>(new Pair<>(weight, goal_value), date));
            }
        } catch (Exception e) {
            Log.e("DB_ERROR", "Error fetching weights for user: " + e.getMessage());
        }

        return weights;
    }

    /**
     * Deletes a specific weight entry for a user.
     *
     * @param userId       The ID of the user.
     * @param weight       The weight value to delete.
     * @param targetWeight The associated goal weight.
     * @param date         The date of the weight entry.
     */
    public void deleteWeightEntry(int userId, int weight, int targetWeight, String date) {
        SQLiteDatabase db = this.getWritableDatabase();
        String whereClause = "user_id = ? AND weight = ? AND goal_value = ? AND date = ?";
        String[] whereArgs = {String.valueOf(userId), String.valueOf(weight), String.valueOf(targetWeight), date};

        db.delete("user_data", whereClause, whereArgs);

    }

    /**
     * Sets the target date for a user's goal.
     *
     * @param userId     The ID of the user.
     * @param targetDate The target date to set.
     * @return True if the update is successful, false otherwise.
     */
    public boolean setTargetDate(int userId, String targetDate) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put("target_date", targetDate);

        int rowsUpdated = db.update("goal", values, "user_id = ?", new String[]{String.valueOf(userId)});

        return rowsUpdated > 0;
    }

    /**
     * Updates the password for a user.
     *
     * @param userId      The ID of the user.
     * @param newPassword The new plaintext password.
     * @return True if the update is successful, false otherwise.
     */
    public boolean updatePassword(int userId, String newPassword) {
        // Hash the new plaintext password
        String hashedPassword = passUtils.hashPassword(newPassword);
        //Log.d("UpdatePassword", "User ID: " + userId);
        //Log.d("UpdatePassword", "Hashed new password: " + hashedPassword);
        // NOTE: Debug statements for testing updatePassword to see if stored correctly.

        ContentValues values = new ContentValues();
        values.put("password", hashedPassword);

        int rowsUpdated = 0;
        try {
            SQLiteDatabase db = this.getWritableDatabase();
            rowsUpdated = db.update("users", values, "id = ?", new String[]{String.valueOf(userId)});
            Log.d("UpdatePassword", "Rows updated: " + rowsUpdated);
        } catch (SQLiteException e) {
            Log.e("UpdatePasswordError", "Error updating password: ", e);
        }
        return rowsUpdated > 0;
    }

}
