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
        resultsGrid->CreateGrid(2, 3);
        resultsGrid->SetColLabelValue(0, "Option Type");
        resultsGrid->SetColLabelValue(1, "Price");
        resultsGrid->SetColLabelValue(2, "Delta");
        resultsGrid->SetRowLabelValue(0, "Call");
        resultsGrid->SetRowLabelValue(1, "Put");
        resultsGrid->SetMinSize(wxSize(300, 100));

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

            double callPrice = mc.price_european_call_option(params);
            double putPrice = mc.price_european_put_option(params);
            double callDelta = mc.calculate_delta(params, true);
            double putDelta = mc.calculate_delta(params, false);

            resultsGrid->SetCellValue(0, 1, wxString::Format("%.4f", callPrice));
            resultsGrid->SetCellValue(1, 1, wxString::Format("%.4f", putPrice));
            resultsGrid->SetCellValue(0, 2, wxString::Format("%.4f", callDelta));
            resultsGrid->SetCellValue(1, 2, wxString::Format("%.4f", putDelta));
        }
        catch (const std::exception& e)
        {
            wxMessageBox(e.what(), "Error", wxOK | wxICON_ERROR);
        }
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

wxIMPLEMENT_APP(MonteCarloApp);
