package com.example.androidapp.utils;

import android.util.Pair;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.example.androidapp.R;

import java.util.ArrayList;
import java.util.List;

public class GridAdapter extends RecyclerView.Adapter<GridAdapter.ViewHolder> {

    public interface OnDeleteClickListener {
        void onDeleteClick(int position); // Pass position to identify the item to delete
    }

    private final List<Pair<Pair<Integer, Integer>, String>> weightEntries;
    private final OnDeleteClickListener onDeleteClickListener;

    public GridAdapter(List<Pair<Pair<Integer, Integer>, String>> weightEntries, OnDeleteClickListener onDeleteClickListener) {
        this.weightEntries = weightEntries != null ? weightEntries : new ArrayList<>();
        this.onDeleteClickListener = onDeleteClickListener;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        final TextView weightTextView;
        final TextView dateTextView;
        final TextView targetWeightTextView;
        final Button deleteButton;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            weightTextView = itemView.findViewById(R.id.weight_text_view);
            dateTextView = itemView.findViewById(R.id.date_text_view);
            targetWeightTextView = itemView.findViewById(R.id.date_target_weight);
            deleteButton = itemView.findViewById(R.id.delete_entry);
        }
    }

    @NonNull
    @Override
    public GridAdapter.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.grid_item_layout, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull GridAdapter.ViewHolder holder, int position) {
        Pair<Pair<Integer, Integer>, String> entry = weightEntries.get(position);
        holder.weightTextView.setText(String.valueOf(entry.first.first));
        holder.targetWeightTextView.setText(String.valueOf(entry.first.second));
        holder.dateTextView.setText(entry.second);

        // Set delete button click listener
        holder.deleteButton.setOnClickListener(v -> onDeleteClickListener.onDeleteClick(position));
    }

    @Override
    public int getItemCount() {
        return weightEntries.size();
    }
}

