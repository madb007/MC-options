#include "volatility_surface.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <algorithm>

VolatilitySurfacePanel::VolatilitySurfacePanel(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, 
                wxSize(800, 600), wxFULL_REPAINT_ON_RESIZE) {
    context_ = std::make_unique<wxGLContext>(this);
    
    Bind(wxEVT_PAINT, &VolatilitySurfacePanel::OnPaint, this);
    Bind(wxEVT_SIZE, &VolatilitySurfacePanel::OnSize, this);
    Bind(wxEVT_MOUSEWHEEL, &VolatilitySurfacePanel::OnMouseWheel, this);
}

void VolatilitySurfacePanel::UpdateSurface(double S, double K, double T, double v) {
    strikes_ = GenerateRange(K * 0.7f, K * 1.3f, 20);
    maturities_ = GenerateRange(T * 0.5f, T * 1.5f, 20);
    vols_ = GenerateVolSurface(strikes_, maturities_, S, K, T, v);
    Refresh();
}

std::vector<float> VolatilitySurfacePanel::GenerateRange(float start, float end, size_t points) {
    std::vector<float> range(points);
    float step = (end - start) / (points - 1);
    std::generate(range.begin(), range.end(), 
        [=, n = 0]() mutable { return start + (n++) * step; });
    return range;
}

std::vector<std::vector<float>> VolatilitySurfacePanel::GenerateVolSurface(
    const std::vector<float>& strikes,
    const std::vector<float>& maturities,
    double S, double K, double T, double v) {
    
    std::vector<std::vector<float>> surface(strikes.size(), 
        std::vector<float>(maturities.size()));
        
    for (size_t i = 0; i < strikes.size(); ++i) {
        for (size_t j = 0; j < maturities.size(); ++j) {
            float moneyness = std::log(strikes[i] / S);
            float timeEffect = std::sqrt(maturities[j] / T);
            surface[i][j] = v * (1.0f + 0.2f * moneyness * moneyness) * timeEffect;
        }
    }
    return surface;
}

void VolatilitySurfacePanel::OnPaint(wxPaintEvent&) {
    wxPaintDC dc(this);
    SetCurrent(*context_);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(rotation_, 1.0f, 0.0f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw surface
    for (size_t i = 0; i < strikes_.size() - 1; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (size_t j = 0; j < maturities_.size(); ++j) {
            float x1 = 2.0f * i / strikes_.size() - 1.0f;
            float x2 = 2.0f * (i + 1) / strikes_.size() - 1.0f;
            float y = 2.0f * j / maturities_.size() - 1.0f;
            
            float vol1 = vols_[i][j];
            float vol2 = vols_[i + 1][j];
            
            glColor4f(0.0f, 0.5f + vol1 * 0.5f, 1.0f - vol1 * 0.5f, 0.7f);
            glVertex3f(x1, y, vol1);
            glColor4f(0.0f, 0.5f + vol2 * 0.5f, 1.0f - vol2 * 0.5f, 0.7f);
            glVertex3f(x2, y, vol2);
        }
        glEnd();
    }
    
    // Draw axes
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();
    
    glFlush();
