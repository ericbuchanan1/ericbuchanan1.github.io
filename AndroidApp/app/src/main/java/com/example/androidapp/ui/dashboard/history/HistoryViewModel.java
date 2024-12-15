package com.example.androidapp.ui.dashboard.history;

import android.app.Application;
import android.util.Pair;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;

import com.example.androidapp.dbHelper.dbConnector;
import com.github.mikephil.charting.data.Entry;

import java.util.ArrayList;
import java.util.List;

/**
 * HistoryViewModel manages the data for HistoryFragment, including loading, deleting,
 * and providing weight entries for charting.
 */
public class HistoryViewModel extends AndroidViewModel {
    private final MutableLiveData<List<Pair<Pair<Integer, Integer>, String>>> weights = new MutableLiveData<>();
    private final dbConnector db;

    /**
     * Constructor initializes the dbConnector.
     *
     * @param application The application context.
     */
    public HistoryViewModel(@NonNull Application application) {
        super(application);
        db = new dbConnector(application.getApplicationContext());
    }

    /**
     * Provides LiveData for the list of weight entries.
     *
     * @return LiveData containing weight data.
     */
    public LiveData<List<Pair<Pair<Integer, Integer>, String>>> getWeights() {
        return weights;
    }

    /**
     * Deletes a specific weight entry and reloads the data.
     *
     * @param userId      The ID of the user.
     * @param weight      The actual weight to delete.
     * @param targetWeight The target weight associated with the entry.
     * @param date        The date of the weight entry.
     */
    public void deleteWeightEntry(int userId, int weight, int targetWeight, String date) {
        db.deleteWeightEntry(userId, weight, targetWeight, date);
        loadWeightsForUser(userId);
    }

    /**
     * Loads weight data for a specific user and updates LiveData.
     *
     * @param userId The ID of the user.
     */
    public void loadWeightsForUser(int userId) {
        List<Pair<Pair<Integer, Integer>, String>> userWeights = db.getUserWeightsWithTarget(userId);
        weights.setValue(userWeights);
    }

    /**
     * Converts actual weight data into chart entries.
     *
     * @return List of Entry objects representing actual weights.
     */
    public List<Entry> getWeightEntries() {
        List<Entry> weightEntries = new ArrayList<>();
        List<Pair<Pair<Integer, Integer>, String>> userWeights = weights.getValue();

        if (userWeights != null) {
            for (int i = 0; i < userWeights.size(); i++) {
                int weight = userWeights.get(i).first.first;
                weightEntries.add(new Entry(i, weight));
            }
        }
        return weightEntries;
    }

    /**
     * Converts target weight data into chart entries.
     *
     * @return List of Entry objects representing target weights.
     */
    public List<Entry> getTargetWeightEntries() {
        List<Entry> targetWeightEntries = new ArrayList<>();
        List<Pair<Pair<Integer, Integer>, String>> userWeights = weights.getValue();

        if (userWeights != null) {
            for (int i = 0; i < userWeights.size(); i++) {
                int targetWeight = userWeights.get(i).first.second;
                targetWeightEntries.add(new Entry(i, targetWeight));
            }
        }
        return targetWeightEntries;
    }

    /**
     * Retrieves date labels for the x-axis of the chart.
     *
     * @return List of date strings.
     */
    public List<String> getDateLabels() {
        List<String> dateLabels = new ArrayList<>();
        List<Pair<Pair<Integer, Integer>, String>> userWeights = weights.getValue();

        if (userWeights != null) {
            for (Pair<Pair<Integer, Integer>, String> entry : userWeights) {
                dateLabels.add(entry.second);
            }
        }
        return dateLabels;
    }
}
