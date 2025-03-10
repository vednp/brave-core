/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.adapters;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;

import org.chromium.brave_wallet.mojom.NetworkInfo;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class NetworkSpinnerAdapter extends BaseAdapter implements SpinnerAdapter {
    private Context context;
    private LayoutInflater inflater;
    private List<NetworkInfo> mNetworkInfos;
    private ExecutorService mExecutor;
    private Handler mHandler;

    public NetworkSpinnerAdapter(Context applicationContext, List<NetworkInfo> networkInfos) {
        this.context = applicationContext;
        inflater = (LayoutInflater.from(applicationContext));
        mNetworkInfos = networkInfos;
        mExecutor = Executors.newSingleThreadExecutor();
        mHandler = new Handler(Looper.getMainLooper());
    }

    public NetworkInfo getNetwork(int position) {
        return mNetworkInfos.get(position);
    }

    @Override
    public int getCount() {
        return mNetworkInfos.size();
    }

    @Override
    public Object getItem(int position) {
        return mNetworkInfos.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    // TODO(samartnik): this one requires review, for now just suppress to force lint checks.
    // Warning: Unconditional layout inflation from view adapter: Should use View Holder pattern
    // (use recycled view passed into this method as the second parameter) for smoother scrolling
    // [ViewHolder]
    @SuppressLint("ViewHolder")
    public View getView(int position, View view, ViewGroup viewGroup) {
        view = inflater.inflate(R.layout.network_spinner_items, null);
        TextView name = view.findViewById(R.id.network_name_text);
        name.setText(Utils.getShortNameOfNetwork(mNetworkInfos.get(position).chainName));
        ImageView networkPicture = view.findViewById(R.id.network_picture);
        networkPicture.setVisibility(View.GONE);
        name.setCompoundDrawablesRelativeWithIntrinsicBounds(null, null, null, null);
        return view;
    }

    @Override
    public View getDropDownView(int position, View view, ViewGroup viewGroup) {
        view = inflater.inflate(R.layout.network_spinner_items, null);
        TextView name = (TextView) view.findViewById(R.id.network_name_text);
        name.setText(mNetworkInfos.get(position).chainName);
        ImageView networkPicture = view.findViewById(R.id.network_picture);
        Utils.setBlockiesBitmapResource(
                mExecutor, mHandler, networkPicture, mNetworkInfos.get(position).chainName, false);

        return view;
    }

    public void setNetworks(List<NetworkInfo> networkInfos) {
        mNetworkInfos = networkInfos;
        notifyDataSetChanged();
    }
}
