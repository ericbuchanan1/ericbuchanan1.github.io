package com.example.androidapp.ui.startPage.login;

import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import com.example.androidapp.MainDashboard;
import com.example.androidapp.databinding.FragmentLoginBinding;
import com.example.androidapp.dbHelper.dbConnector;

/**
 * LoginFragment handles user authentication by allowing users to log in with their credentials.
 * It interacts with the dbConnector to verify user credentials and navigates to the MainDashboard upon successful login.
 */
public class LoginFragment extends Fragment {

    private FragmentLoginBinding binding; // ViewBinding for the fragment's layout
    private dbConnector dbConnection;     // Database connector for handling login operations

    /**
     * Called to have the fragment instantiate its user interface view.
     *
     * @param inflater           The LayoutInflater object that can be used to inflate any views in the fragment.
     * @param container          If non-null, this is the parent view that the fragment's UI should be attached to.
     * @param savedInstanceState If non-null, this fragment is being re-constructed from a previous saved state.
     * @return The root View for the fragment's UI.
     */
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {

        // Inflate the layout using ViewBinding
        binding = FragmentLoginBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        // Initialize dbConnector with the current context
        dbConnection = new dbConnector(requireContext());

        // Reference to the Login button and set its click listener
        final Button login_button = binding.buttonLogin;
        login_button.setOnClickListener(v -> attemptLogin());

        return root; // Return the root view of the binding
    }

    /**
     * Attempts to log in the user by validating the entered credentials.
     * On successful login, navigates to the MainDashboard. Displays appropriate Toast messages based on the result.
     */
    public void attemptLogin(){
        // Retrieve username and password from input fields
        String user = binding.editLoginUsername.getText().toString();
        String pass = binding.editLoginUsername.getText().toString(); // **Potential Issue:** Should this be editLoginPassword?

        // Attempt to log in using the dbConnector
        int result = dbConnection.loginUser(user, pass);

        // Handle the result of the login attempt
        switch (result){
            case 1: // Successful login
                Toast.makeText(requireContext(), "Success! Logging you in.", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(requireContext(), MainDashboard.class);
                startActivity(intent);
                break;
            case -1: // Invalid credentials
            case -2:
                Toast.makeText(requireContext(), "Invalid credentials. Please try again", Toast.LENGTH_SHORT).show();
                break;
            case -3: // Database error
                Toast.makeText(requireContext(), "Database error. Please try again later", Toast.LENGTH_SHORT).show();
                break;
            default: // Unexpected error
                Toast.makeText(requireContext(), "Error. Please report issue to developer.", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Called when the view previously created by onCreateView() has been detached from the fragment.
     * Cleans up the binding to prevent memory leaks.
     */
    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

}
