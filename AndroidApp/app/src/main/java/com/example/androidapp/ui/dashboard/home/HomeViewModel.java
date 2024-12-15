package com.example.androidapp.ui.dashboard.home;

import android.app.Application;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;

import com.example.androidapp.dbHelper.dbConnector;

/**
 * HomeViewModel manages the data operations for HomeFragment,
 * including setting and retrieving target dates and fetching the current user ID.
 */
public class HomeViewModel extends AndroidViewModel {
    private final dbConnector db; // Database connector instance for data operations

    /**
     * Constructor initializes the dbConnector with the application context.
     *
     * @param application The application context.
     */
    public HomeViewModel(@NonNull Application application) {
        super(application);
        db = new dbConnector(application.getApplicationContext());
    }

    /**
     * Sets the target date for the user.
     *
     * @param userId     The ID of the user.
     * @param targetDate The target date in "MM-DD-YYYY" format.
     * @return True if the operation was successful, false otherwise.
     */
    public boolean setTargetDate(int userId, String targetDate) {
        return db.setTargetDate(userId, targetDate); // Call the method in dbConnector
    }

    /**
     * Retrieves the most recent target date for the user.
     *
     * @param userId The ID of the user.
     * @return The most recent target date as a String, or null if not set.
     */
    public String getMostRecentTargetDate(int userId) {
        return db.getMostRecentTargetDate(userId); // Fetch the latest target date from dbConnector
    }

    /**
     * Retrieves the current user ID from the session.
     *
     * @return The current user's ID, or -1 if not found.
     */
    public int getCurrentUserId() {
        return db.getCurrentUserID(); // Fetch current user ID from dbConnector
    }
}
