package com.example.androidapp.ui.dashboard.settings;

import android.Manifest;
import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;

import com.example.androidapp.R;
import com.example.androidapp.databinding.FragmentSettingsBinding;
import com.example.androidapp.dbHelper.dbConnector;

/**
 * SettingsFragment allows users to toggle dark mode, change their password,
 * and request SMS permissions. It manages user preferences and interacts with the database.
 */
public class SettingsFragment extends Fragment {

    private static final int SMS_PERMISSION_CODE = 100; // Request code for SMS permission
    private FragmentSettingsBinding binding;            // ViewBinding for the fragment's layout

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
        // Initialize ViewBinding
        binding = FragmentSettingsBinding.inflate(inflater, container, false);

        // Access SharedPreferences to retrieve the current dark mode setting
        SharedPreferences sharedPreferences = requireActivity().getSharedPreferences("user_prefs", Context.MODE_PRIVATE);
        boolean isDarkMode = sharedPreferences.getBoolean("dark_mode", false);

        // Set the switch state based on the retrieved dark mode preference
        binding.switchDarkMode.setChecked(isDarkMode);

        // Listener to handle dark mode toggle
        binding.switchDarkMode.setOnCheckedChangeListener((buttonView, isChecked) -> {
            // Apply the selected night mode
            AppCompatDelegate.setDefaultNightMode(isChecked
                    ? AppCompatDelegate.MODE_NIGHT_YES
                    : AppCompatDelegate.MODE_NIGHT_NO);

            // Save the updated dark mode preference
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putBoolean("dark_mode", isChecked);
            editor.apply();
        });

        // Set up click listener for the "Change Password" button
        binding.buttonChangePassword.setOnClickListener(v -> showChangePasswordDialog());

        // Set up click listener for the "Allow Permissions" button
        binding.buttonAllowPermissions.setOnClickListener(v -> checkAndRequestSmsPermission());

        return binding.getRoot(); // Return the root view of the binding
    }

    /**
     * Displays a dialog to allow the user to change their password.
     * Validates input and updates the password in the database.
     */
    private void showChangePasswordDialog() {
        // Build the AlertDialog
        AlertDialog.Builder builder = new AlertDialog.Builder(requireContext());
        builder.setTitle("Change Password");

        // Inflate the custom dialog layout
        View dialogView = LayoutInflater.from(getContext()).inflate(R.layout.dialog_change_password, null);
        builder.setView(dialogView);

        // Reference to the EditText for the new password
        EditText editTextNewPassword = dialogView.findViewById(R.id.editTextNewPassword);

        // Set up the Cancel button
        builder.setNegativeButton("Cancel", (dialog, which) -> dialog.dismiss());

        // Set up the OK button with validation
        builder.setPositiveButton("OK", (dialog, which) -> {
            String newPassword = editTextNewPassword.getText().toString().trim();

            // Validate that the password is not empty
            if (newPassword.isEmpty()) {
                Toast.makeText(getContext(), "Password cannot be empty", Toast.LENGTH_SHORT).show();
                return;
            }

            // Update the password in the database
            updatePassword(newPassword);
        });

        // Create and show the dialog
        builder.create().show();
    }

    /**
     * Checks if SMS permission is granted. If not, requests the permission from the user.
     * Provides feedback based on the permission status.
     */
    private void checkAndRequestSmsPermission() {
        // Check if SEND_SMS permission is already granted
        if (ContextCompat.checkSelfPermission(requireContext(), Manifest.permission.SEND_SMS)
                != PackageManager.PERMISSION_GRANTED) {
            // Permission is not granted, request it
            ActivityCompat.requestPermissions(requireActivity(),
                    new String[]{Manifest.permission.SEND_SMS},
                    SMS_PERMISSION_CODE);
        } else {
            // Permission is already granted
            Toast.makeText(getContext(), "SMS permission is already granted", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Updates the user's password in the database.
     *
     * @param newPassword The new password entered by the user.
     */
    private void updatePassword(String newPassword) {
        // Initialize the database connector
        dbConnector db = new dbConnector(getContext());

        // Retrieve the current user's ID
        int userId = db.getCurrentUserID();
        //Log.d("UpdatePassword", "Current user ID: " + userId);
        // Note: here for debugging

        // Check if a user is logged in
        if (userId == -1) {
            Toast.makeText(getContext(), "No user logged in", Toast.LENGTH_SHORT).show();
            return;
        }

        // Attempt to update the password in the database
        boolean isUpdated = db.updatePassword(userId, newPassword);
        if (isUpdated) {
            Toast.makeText(getContext(), "Password updated successfully", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(getContext(), "Failed to update password", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Called when the view previously created by onCreateView() has been detached from the fragment.
     * Cleans up the binding to prevent memory leaks.
     */
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null; // Nullify the binding reference
    }
}
