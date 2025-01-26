#include "MonteCarloFrame.h"

MonteCarloFrame::MonteCarloFrame() 
    : wxFrame(nullptr, wxID_ANY, "Multi-Method Option Pricer") {
    notebook_ = std::make_unique<wxNotebook>(this, wxID_ANY);
    
    auto pricing_panel = new wxPanel(notebook_.get());
    CreatePricingControls(pricing_panel);
    
    vol_panel_ = std::make_unique<VolatilitySurfacePanel>(notebook_.get());
    
    notebook_->AddPage(pricing_panel, "Option Pricing");
    notebook_->AddPage(vol_panel_.get(), "Volatility Surface");
    
    auto sizer = std::make_unique<wxBoxSizer>(wxVERTICAL);
    sizer->Add(notebook_.get(), 1, wxEXPAND);
    SetSizer(sizer.release());
}

void MonteCarloFrame::CreatePricingControls(wxPanel* panel) {
    wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);
    stockPriceCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Stock Price (S):", "100");
    strikePriceCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Strike Price (K):", "100");
    riskFreeRateCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Risk-free Rate (r):", "0.05");
    volatilityCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Volatility (σ):", "0.2");
    timeToMaturityCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Time to Maturity (T):", "1");
    numSamplesCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Number of Samples:", "1000000");
    numThreadsCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Number of Threads:", "4");

    wxButton* calculateButton = new wxButton(panel, wxID_ANY, "Calculate");
    calculateButton->Bind(wxEVT_BUTTON, &MonteCarloFrame::OnCalculate, this);
    inputSizer->Add(calculateButton, 0, wxALL, 5);

    resultsGrid = new wxGrid(panel, wxID_ANY);
    resultsGrid->CreateGrid(6, 3);
    resultsGrid->SetColLabelValue(0, "Metric");
    resultsGrid->SetColLabelValue(1, "Call");
    resultsGrid->SetColLabelValue(2, "Put");
    
    wxArrayString rowLabels = {"Monte-Carlo Price", "Black-Scholes Price", 
                              "Delta", "Gamma", "Theta", "Vega"};
    for (size_t i = 0; i < rowLabels.size(); ++i) {
        resultsGrid->SetCellValue(i, 0, rowLabels[i]);
    }
    
    resultsGrid->AutoSizeColumns();
    resultsGrid->SetMinSize(wxSize(375, 275));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(inputSizer, 0, wxALL, 10);
    mainSizer->Add(resultsGrid, 1, wxEXPAND | wxALL, 10);

    panel->SetSizer(mainSizer);
}

wxTextCtrl* MonteCarloFrame::CreateLabeledTextCtrl(wxWindow* parent, wxSizer* sizer, 
                                                  const wxString& label, const wxString& value) {
    wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);
    rowSizer->Add(new wxStaticText(parent, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxTextCtrl* ctrl = new wxTextCtrl(parent, wxID_ANY, value);
    rowSizer->Add(ctrl, 1);
    sizer->Add(rowSizer, 0, wxEXPAND | wxALL, 5);
    return ctrl;
}

void MonteCarloFrame::OnCalculate(wxCommandEvent& event) {
    try {
        double S = wxAtof(stockPriceCtrl->GetValue());
        double K = wxAtof(strikePriceCtrl->GetValue());
        double r = wxAtof(riskFreeRateCtrl->GetValue());
        double v = wxAtof(volatilityCtrl->GetValue());
        double T = wxAtof(timeToMaturityCtrl->GetValue());
        long long numSamples = wxAtoi(numSamplesCtrl->GetValue());
        int numThreads = wxAtoi(numThreadsCtrl->GetValue());

        FinanceMonteCarlo mc(numThreads);
        OptionParams params{S, K, T, r, v, numSamples};

        double callPrice = mc.price_european_option(params, true);
        double putPrice = mc.price_european_option(params, false);
        double callPriceBS = mc.black_scholes_price(params, true);
        double putPriceBS = mc.black_scholes_price(params, false);
        double callDelta = mc.black_scholes_delta(params, true);
        double putDelta = mc.black_scholes_delta(params, false);
        double gamma = mc.black_scholes_gamma(params);
        double callTheta = mc.black_scholes_theta(params, true);
        double putTheta = mc.black_scholes_theta(params, false);
        double vega = mc.black_scholes_vega(params);

        UpdateGridCell(0, 1, callPrice);
        UpdateGridCell(0, 2, putPrice);
        UpdateGridCell(1, 1, callPriceBS);
        UpdateGridCell(1, 2, putPriceBS);
        UpdateGridCell(2, 1, callDelta);
        UpdateGridCell(2, 2, putDelta);
        UpdateGridCell(3, 1, gamma);
        UpdateGridCell(3, 2, gamma);
        UpdateGridCell(4, 1, callTheta);
        UpdateGridCell(4, 2, putTheta);
        UpdateGridCell(5, 1, vega);
        UpdateGridCell(5, 2, vega);

        resultsGrid->AutoSizeColumns();
        
        vol_panel_->UpdateSurface(S, K, T, v);
    }
    catch (const std::exception& e) {
        wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
    }
}

void MonteCarloFrame::UpdateGridCell(int row, int col, double value) {
    resultsGrid->SetCellValue(row, col, wxString::Format("%.6f", value));
}
