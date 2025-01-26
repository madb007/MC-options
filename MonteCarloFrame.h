#pragma once
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/notebook.h>
#include "FinanceMonteCarlo.h"
#include "volatility_surface.h"

class MonteCarloFrame : public wxFrame {
public:
    MonteCarloFrame();

private:
    void CreatePricingControls(wxPanel* panel);
    wxTextCtrl* CreateLabeledTextCtrl(wxWindow* parent, wxSizer* sizer, 
                                     const wxString& label, const wxString& value);
    void OnCalculate(wxCommandEvent& event);
    void UpdateGridCell(int row, int col, double value);

    std::unique_ptr<wxNotebook> notebook_;
    std::unique_ptr<VolatilitySurfacePanel> vol_panel_;
    wxTextCtrl *stockPriceCtrl, *strikePriceCtrl, *riskFreeRateCtrl, 
               *volatilityCtrl, *timeToMaturityCtrl, *numSamplesCtrl, *numThreadsCtrl;
    wxGrid* resultsGrid;
};
