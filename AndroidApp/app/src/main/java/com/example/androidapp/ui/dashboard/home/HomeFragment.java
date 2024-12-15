package com.example.androidapp.ui.dashboard.home;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.example.androidapp.databinding.FragmentHomeBinding;

import java.util.Locale;
import java.util.Objects;

/**
 * HomeFragment allows users to view and set their target dates.
 * It displays the most recent target date and provides a CalendarView for selecting new dates.
 */
public class HomeFragment extends Fragment {

    private HomeViewModel mViewModel;                // ViewModel for managing UI-related data
    private FragmentHomeBinding binding;             // ViewBinding for the fragment's layout
    private String selectedDate;                     // Holds the selected date in "MM-DD-YYYY" format

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
        mViewModel = new ViewModelProvider(this).get(HomeViewModel.class);
        binding = FragmentHomeBinding.inflate(inflater, container, false);

        // Retrieve current user ID from ViewModel
        int userId = mViewModel.getCurrentUserId();

        // Get the most recent target date for the user and display it
        String recentTargetDate = mViewModel.getMostRecentTargetDate(userId);
        if (recentTargetDate != null) {
            binding.targetDate.setText(recentTargetDate);
        }

        // Set up CalendarView listener to capture the selected date
        Objects.requireNonNull(binding.calendarView).setOnDateChangeListener((view, year, month, dayOfMonth) -> {
            // Format the selected date as "MM-DD-YYYY"
            selectedDate = String.format(Locale.getDefault(), "%02d-%02d-%04d", month + 1, dayOfMonth, year);
        });

        // Set the target date when the button is clicked
        binding.buttonSetTargetDate.setOnClickListener(v -> setTargetDate());

        return binding.getRoot();
    }

    /**
     * Sets the target date for the user based on the selected date from the CalendarView.
     * Displays a Toast message indicating success or failure.
     */
    private void setTargetDate() {
        // Retrieve current user ID
        int userId = mViewModel.getCurrentUserId();

        if (selectedDate != null) {
            // Attempt to set the target date using the ViewModel
            boolean success = mViewModel.setTargetDate(userId, selectedDate);
            if (success) {
                // Inform the user of successful date setting
                Toast.makeText(getContext(),
                        "Target date set successfully for " + selectedDate,
                        Toast.LENGTH_SHORT).show();
            } else {
                // Inform the user of failure to set the date
                Toast.makeText(getContext(), "Failed to set target date",
                        Toast.LENGTH_SHORT).show();
            }
        } else {
            // Prompt the user to select a date if none has been selected
            Toast.makeText(getContext(), "Please select a date",
                    Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Called when the view previously created by onCreateView() has been detached from the fragment.
     * Cleans up the binding to prevent memory leaks.
     */
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null; // Prevent memory leaks by clearing the binding reference
    }
}
