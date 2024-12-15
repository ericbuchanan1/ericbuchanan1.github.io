package com.example.androidapp.ui.dashboard.tracking;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.example.androidapp.databinding.FragmentTrackingBinding;
import com.example.androidapp.dbHelper.dbConnector;

/**
 * TrackingFragment allows users to set and view their goal weight and add daily weight entries.
 * It interacts with the TrackingViewModel to manage data operations.
 */
public class TrackingFragment extends Fragment {

    private TrackingViewModel mViewModel;                // ViewModel for managing tracking data
    private FragmentTrackingBinding binding;             // ViewBinding for the fragment's layout
    private dbConnector db;                               // Database connector for data operations
    private int userId;                                   // Current user's ID

    /**
     * Called to have the fragment instantiate its user interface view.
     *
     * @param inflater           The LayoutInflater object that can be used to inflate any views in the fragment.
     * @param container          If non-null, this is the parent view that the fragment's UI should be attached to.
     * @param savedInstanceState If non-null, this fragment is being re-constructed from a previous saved state.
     * @return The root View for the fragment's UI.
     */
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        // Initialize ViewModel and ViewBinding
        mViewModel = new ViewModelProvider(this).get(TrackingViewModel.class);
        binding = FragmentTrackingBinding.inflate(inflater, container, false);

        // Initialize dbConnector and retrieve current user ID
        db = new dbConnector(getContext());
        userId = db.getCurrentUserID();

        // Load the user's current goal weight
        mViewModel.loadGoalWeight(db, userId);

        // Observe changes to the goal weight and update the UI accordingly
        mViewModel.getGoalWeight().observe(getViewLifecycleOwner(), goalWeight -> {
            if (goalWeight != null) {
                binding.textGoalWeight.setText(String.valueOf(goalWeight));
            }
        });

        // Set up the button to set a new goal weight
        binding.buttonSetGoal.setOnClickListener(view -> {
            String inputGoal = binding.editEnterGoalWeight.getText().toString();
            if (!inputGoal.isEmpty()) {
                try {
                    int goalWeight = Integer.parseInt(inputGoal);
                    if (mViewModel.setGoalWeight(db, userId, goalWeight)) {
                        Toast.makeText(getContext(), "Goal weight set successfully", Toast.LENGTH_SHORT).show();
                        binding.textGoalWeight.setText(String.valueOf(goalWeight)); // Update displayed goal
                    } else {
                        Toast.makeText(getContext(), "Error setting goal weight", Toast.LENGTH_SHORT).show();
                    }
                } catch (NumberFormatException e) {
                    Toast.makeText(getContext(), "Please enter a valid number", Toast.LENGTH_SHORT).show();
                }
            } else {
                Toast.makeText(getContext(), "Enter a valid goal weight", Toast.LENGTH_SHORT).show();
            }
        });

        // Set up the button to add a daily weight entry
        binding.buttonAddWeight.setOnClickListener(view -> {
            String inputDailyWeight = binding.editEnterWeight.getText().toString();
            if (!inputDailyWeight.isEmpty()) {
                try {
                    int dailyWeight = Integer.parseInt(inputDailyWeight);
                    if (mViewModel.addDailyWeight(db, userId, dailyWeight)) {
                        Toast.makeText(getContext(), "Daily weight added successfully", Toast.LENGTH_SHORT).show();
                        binding.editEnterWeight.setText(""); // Clear input field after successful addition
                    } else {
                        Toast.makeText(getContext(), "Error adding daily weight", Toast.LENGTH_SHORT).show();
                    }
                } catch (NumberFormatException e) {
                    Toast.makeText(getContext(), "Please enter a valid number", Toast.LENGTH_SHORT).show();
                }
            } else {
                Toast.makeText(getContext(), "Enter a valid daily weight", Toast.LENGTH_SHORT).show();
            }
        });

        return binding.getRoot();
    }

    /**
     * Called when the view previously created by onCreateView() has been detached from the fragment.
     * Cleans up the binding to prevent memory leaks.
     */
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null; // Clear the binding reference
    }
}
