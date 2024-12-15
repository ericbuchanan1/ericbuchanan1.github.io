package com.example.androidapp.ui.startPage.register;

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
import com.example.androidapp.databinding.FragmentRegisterBinding;
import com.example.androidapp.dbHelper.dbConnector;

/**
 * RegisterFragment handles user registration by collecting user input,
 * validating it, and interacting with the database to create new user accounts.
 */
public class RegisterFragment extends Fragment {

    private FragmentRegisterBinding binding; // ViewBinding for the fragment's layout
    private dbConnector dbConnection;         // Database connector for handling registration operations

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
        // Initialize ViewModel (commented out as it's not used in the current implementation)
        // RegisterViewModel registerViewModel = new ViewModelProvider(this).get(RegisterViewModel.class);

        // Inflate the layout using ViewBinding
        binding = FragmentRegisterBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        // Initialize dbConnector with the current context
        dbConnection = new dbConnector(requireContext());


        // Reference to the Register button and set its click listener
        final Button register_button = binding.buttonRegister;
        //final Button verify_button = binding.buttonVerify; // For phone number verification (Not implemented yet)

        // Set click listener for the Register button to initiate registration process
        register_button.setOnClickListener(v -> attemptRegister());
        //verify_button.setOnClickListener(v -> attemptVerification()); // For phone number verification


        return root; // Return the root view of the binding
    }

    /**
     * Initiates the registration process by collecting user input,
     * validating it, and interacting with the database to create a new user account.
     */
    private void attemptRegister(){
        // Retrieve user input from the input fields
        String new_username = binding.editRegisterUsername.getText().toString();
        String enc_pass = binding.editRegisterPassword.getText().toString();
        String reg_group = binding.editRegisterGroupNumber.getText().toString();
        String phone_num = binding.editRegisterPhone.getText().toString();

        // Attempt to register the user using the dbConnector
        int result = dbConnection.registerUser(new_username, enc_pass, phone_num, reg_group);

        // Handle the result of the registration attempt
        switch(result) {
            case 1: // Successful registration
                Toast.makeText(requireContext(), "Thanks for registering. Logging you in.", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(requireContext(), MainDashboard.class);
                startActivity(intent); // Navigate to the main dashboard
                break;
            case -1: // Username already taken
                Toast.makeText(requireContext(), "Username has been taken. Please try again.", Toast.LENGTH_SHORT).show();
                break;
            case -3: // Database error
                Toast.makeText(requireContext(), "Oh no, there was a database error. Please try again.", Toast.LENGTH_SHORT).show();
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
