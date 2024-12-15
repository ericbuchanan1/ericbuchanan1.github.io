package com.example.androidapp.ui.dashboard.tracking;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

import com.example.androidapp.dbHelper.dbConnector;

/**
 * TrackingViewModel manages the data operations for TrackingFragment,
 * including loading, setting goal weights, and adding daily weight entries.
 */
public class TrackingViewModel extends ViewModel {

    // LiveData holding the user's current goal weight
    private final MutableLiveData<Integer> goalWeight = new MutableLiveData<>();

    /**
     * Provides LiveData for observing the user's goal weight.
     *
     * @return LiveData containing the goal weight.
     */
    public LiveData<Integer> getGoalWeight() {
        return goalWeight;
    }

    /**
     * Loads the current goal weight for the specified user from the database
     * and updates the LiveData if a valid goal weight is found.
     *
     * @param db     The database connector for performing data operations.
     * @param userId The unique identifier of the user.
     */
    public void loadGoalWeight(dbConnector db, int userId) {
        int currentGoalWeight = db.getMostRecentGoalWeight(userId);
        if (currentGoalWeight != -1) { // Check if a valid goal weight exists
            goalWeight.setValue(currentGoalWeight); // Update LiveData with the retrieved goal weight
        }
    }

    /**
     * Sets a new goal weight for the specified user in the database.
     * If the operation is successful, updates the LiveData with the new goal weight.
     *
     * @param db         The database connector for performing data operations.
     * @param userId     The unique identifier of the user.
     * @param goalWeight The new goal weight to be set.
     * @return True if the goal weight was successfully set; otherwise, false.
     */
    public boolean setGoalWeight(dbConnector db, int userId, int goalWeight) {
        boolean success = db.setGoalWeight(userId, goalWeight); // Attempt to set the new goal weight in the database
        if (success) {
            this.goalWeight.setValue(goalWeight); // Update LiveData if the operation was successful
        }
        return success;
    }

    /**
     * Adds a daily weight entry for the specified user in the database.
     *
     * @param db     The database connector for performing data operations.
     * @param userId The unique identifier of the user.
     * @param weight The daily weight to be added.
     * @return True if the daily weight was successfully added; otherwise, false.
     */
    public boolean addDailyWeight(dbConnector db, int userId, int weight) {
        return db.addDailyWeight(userId, weight); // Delegate the operation to dbConnector and return the result
    }
}
