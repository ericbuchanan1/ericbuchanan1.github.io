package com.example.androidapp.ui.dashboard.history;

import android.graphics.Color;
import android.os.Bundle;
import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.GridLayoutManager;

import com.example.androidapp.databinding.FragmentHistoryBinding;
import com.example.androidapp.dbHelper.dbConnector;
import com.example.androidapp.utils.GridAdapter;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.Legend;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.IndexAxisValueFormatter;

import java.util.ArrayList;
import java.util.List;


/**
 * HistoryFragment displays the user's weight history using a LineChart and a RecyclerView.
 * It allows viewing and deleting specific weight entries.
 */
public class HistoryFragment extends Fragment implements GridAdapter.OnDeleteClickListener {

    private HistoryViewModel mViewModel;                // ViewModel for managing UI-related data
    private FragmentHistoryBinding binding;             // ViewBinding for the fragment's layout
    private GridAdapter adapter;                        // Adapter for the RecyclerView
    private LineChart lineChart;                        // LineChart to display weight trends
    private int userId;                                 // Current user's ID

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
        mViewModel = new ViewModelProvider(this).get(HistoryViewModel.class);
        binding = FragmentHistoryBinding.inflate(inflater, container, false);

        // Retrieve current user ID from the database
        dbConnector db = new dbConnector(getContext());
        userId = db.getCurrentUserID();

        // Reference to the LineChart in the layout
        lineChart = binding.lineChart;

        // Initialize RecyclerView with GridAdapter
        adapter = new GridAdapter(new ArrayList<>(), this);
        binding.dataGrid.setLayoutManager(new GridLayoutManager(getContext(), 1));
        binding.dataGrid.setAdapter(adapter);

        // Load user's weight data
        mViewModel.loadWeightsForUser(userId);

        // Observe LiveData for weight changes and update UI accordingly
        mViewModel.getWeights().observe(getViewLifecycleOwner(), newWeights -> {
            if (newWeights != null) {
                adapter = new GridAdapter(newWeights, this); // Update adapter with new data
                binding.dataGrid.setAdapter(adapter);         // Refresh data in RecyclerView
                updateChart();                               // Refresh LineChart
            }
        });

        return binding.getRoot();
    }

    /**
     * Updates the LineChart with the latest weight and target weight data.
     */
    private void updateChart() {
        List<Entry> weightEntries = mViewModel.getWeightEntries();
        List<Entry> targetWeightEntries = mViewModel.getTargetWeightEntries();
        List<String> dateLabels = mViewModel.getDateLabels();

        // Create LineData with actual and target weight datasets
        LineData lineData = getLineData(weightEntries, targetWeightEntries);

        // Set data to the LineChart
        lineChart.setData(lineData);

        // Configure X-axis with date labels
        XAxis xAxis = lineChart.getXAxis();
        xAxis.setValueFormatter(new IndexAxisValueFormatter(dateLabels));
        xAxis.setGranularity(1f); // Ensure x-axis labels are shown for each data point
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);

        // Configure Legend to display dataset labels
        Legend legend = lineChart.getLegend();
        legend.setEnabled(true);
        legend.setTextSize(16f);

        // Refresh the chart to display updated data
        lineChart.invalidate();
    }

    /**
     * Creates LineData with actual and target weight datasets.
     *
     * @param weightEntries       List of actual weight entries.
     * @param targetWeightEntries List of target weight entries.
     * @return Configured LineData object.
     */
    @NonNull
    private static LineData getLineData(List<Entry> weightEntries, List<Entry> targetWeightEntries) {
        LineDataSet weightDataSet = new LineDataSet(weightEntries, "Actual Weight");
        weightDataSet.setColor(Color.BLUE);
        weightDataSet.setCircleColor(Color.BLUE);

        LineDataSet targetWeightDataSet = new LineDataSet(targetWeightEntries, "Target Weight");
        targetWeightDataSet.setColor(Color.RED);
        targetWeightDataSet.setCircleColor(Color.RED);

        return new LineData(weightDataSet, targetWeightDataSet);
    }

    /**
     * Handles the deletion of a weight entry when the delete button is clicked.
     *
     * @param position The position of the item in the RecyclerView.
     */
    @Override
    public void onDeleteClick(int position) {
        List<Pair<Pair<Integer, Integer>, String>> weights = mViewModel.getWeights().getValue();
        if (weights == null || position >= weights.size()) {
            return;
        }

        Pair<Pair<Integer, Integer>, String> entry = weights.get(position);
        int weight = entry.first.first;
        int targetWeight = entry.first.second;
        String date = entry.second;

        mViewModel.deleteWeightEntry(userId, weight, targetWeight, date);
        mViewModel.loadWeightsForUser(userId);
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
