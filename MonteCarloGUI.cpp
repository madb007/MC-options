#include <wx/wx.h>
#include <wx/grid.h>
#include "FinanceMonteCarlo.h"

class MonteCarloFrame : public wxFrame
{
public:
    MonteCarloFrame() : wxFrame(nullptr, wxID_ANY, "Monte Carlo Option Pricer")
    {
        wxPanel* panel = new wxPanel(this);

        // Create input fields
        wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);
        stockPriceCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Stock Price (S):", "100");
        strikePriceCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Strike Price (K):", "100");
        riskFreeRateCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Risk-free Rate (r):", "0.05");
        volatilityCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Volatility (σ):", "0.2");
        timeToMaturityCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Time to Maturity (T):", "1");
        numSamplesCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Number of Samples:", "1000000");
        numThreadsCtrl = CreateLabeledTextCtrl(panel, inputSizer, "Number of Threads:", "4");

        // Create button
        wxButton* calculateButton = new wxButton(panel, wxID_ANY, "Calculate");
        calculateButton->Bind(wxEVT_BUTTON, &MonteCarloFrame::OnCalculate, this);
        inputSizer->Add(calculateButton, 0, wxALL, 5);

        // Create results grid
        resultsGrid = new wxGrid(panel, wxID_ANY);
        resultsGrid->CreateGrid(5, 3);  // 5 rows (Price + 4 Greeks), 3 columns (Metric, Call, Put)
        resultsGrid->SetColLabelValue(0, "Metric");
        resultsGrid->SetColLabelValue(1, "Call");
        resultsGrid->SetColLabelValue(2, "Put");
        
        wxArrayString rowLabels = {"Price", "Delta", "Gamma", "Theta", "Vega"};
        for (size_t i = 0; i < rowLabels.size(); ++i) {
            resultsGrid->SetCellValue(i, 0, rowLabels[i]);
        }
        
        resultsGrid->SetDefaultColSize(87,false);
        resultsGrid->SetDefaultRowSize(84,false);
        resultsGrid->SetMinSize(wxSize(340, 450));

        // Main sizer
        wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);
        mainSizer->Add(inputSizer, 0, wxALL, 10);
        mainSizer->Add(resultsGrid, 1, wxEXPAND | wxALL, 10);

        panel->SetSizer(mainSizer);
        mainSizer->SetSizeHints(this);
    }

private:
    wxTextCtrl* CreateLabeledTextCtrl(wxWindow* parent, wxSizer* sizer, const wxString& label, const wxString& value)
    {
        wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);
        rowSizer->Add(new wxStaticText(parent, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
        wxTextCtrl* ctrl = new wxTextCtrl(parent, wxID_ANY, value);
        rowSizer->Add(ctrl, 1);
        sizer->Add(rowSizer, 0, wxEXPAND | wxALL, 5);
        return ctrl;
    }

    void OnCalculate(wxCommandEvent& event)
    {
        try
        {
            double S = std::stod(stockPriceCtrl->GetValue().ToStdString());
            double K = std::stod(strikePriceCtrl->GetValue().ToStdString());
            double r = std::stod(riskFreeRateCtrl->GetValue().ToStdString());
            double v = std::stod(volatilityCtrl->GetValue().ToStdString());
            double T = std::stod(timeToMaturityCtrl->GetValue().ToStdString());
            long long numSamples = std::stoll(numSamplesCtrl->GetValue().ToStdString());
            int numThreads = std::stoi(numThreadsCtrl->GetValue().ToStdString());

            FinanceMonteCarlo mc(numThreads);
            OptionParams params{S, K, r, v, T, numSamples};

            // Calculate metrics
            double callPrice = mc.price_european_call_option(params);
            double putPrice = mc.price_european_put_option(params);
            double callDelta = mc.calculate_delta(params, true);
            double putDelta = mc.calculate_delta(params, false);
            double gamma = mc.calculate_gamma(params);
            double callTheta = mc.calculate_theta(params, true);
            double putTheta = mc.calculate_theta(params, false);
            double vega = mc.calculate_vega(params);

            // Update the grid with results
            UpdateGridCell(0, 1, callPrice);
            UpdateGridCell(0, 2, putPrice);
            UpdateGridCell(1, 1, callDelta);
            UpdateGridCell(1, 2, putDelta);
            UpdateGridCell(2, 1, gamma);
            UpdateGridCell(2, 2, gamma);
            UpdateGridCell(3, 1, callTheta);
            UpdateGridCell(3, 2, putTheta);
            UpdateGridCell(4, 1, vega);
            UpdateGridCell(4, 2, vega);

            resultsGrid->AutoSizeColumns();
        }
        catch (const std::exception& e)
        {
            wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
        }
    }

    void UpdateGridCell(int row, int col, double value)
    {
        resultsGrid->SetCellValue(row, col, wxString::Format("%.6f", value));
    }

    wxTextCtrl *stockPriceCtrl, *strikePriceCtrl, *riskFreeRateCtrl, *volatilityCtrl, 
                *timeToMaturityCtrl, *numSamplesCtrl, *numThreadsCtrl;
    wxGrid* resultsGrid;
};

class MonteCarloApp : public wxApp
{
public:
    bool OnInit() override
    {
        MonteCarloFrame* frame = new MonteCarloFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MonteCarloApp);    // ... (same as before)
