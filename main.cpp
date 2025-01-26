#include <wx/wx.h>
#include "MonteCarloFrame.h"

class MonteCarloApp : public wxApp {
public:
    bool OnInit() override {
        MonteCarloFrame* frame = new MonteCarloFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MonteCarloApp);
