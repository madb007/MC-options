#pragma once
#include <wx/glcanvas.h>
#include <memory>
#include <vector>

class VolatilitySurfacePanel : public wxGLCanvas {
public:
    explicit VolatilitySurfacePanel(wxWindow* parent);
    void UpdateSurface(double S, double K, double T, double v);

private:
    std::unique_ptr<wxGLContext> context_;
    std::vector<float> strikes_;
    std::vector<float> maturities_;
    std::vector<std::vector<float>> vols_;
    float rotation_{45.0f};
    
    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnMouseWheel(wxMouseEvent& evt);
    
    std::vector<float> GenerateRange(float start, float end, size_t points);
    std::vector<std::vector<float>> GenerateVolSurface(
        const std::vector<float>& strikes,
        const std::vector<float>& maturities,
        double S, double K, double T, double v);
};
