#pragma once
#include <wx/wx.h>
#include <string>

class MainCanvas;

class Port {
private:
    MainCanvas* canvas = nullptr;
public:
    std::string id;
    wxPoint2DDouble relativePos;
    mutable wxPoint cachedScreenPos;

    /** @brief 생성자 */
    Port(MainCanvas* canvas, const std::string& id, const wxPoint2DDouble& relPos);

    /** @brief 포트의 화면 좌표 계산 및 캐시 */
    wxPoint GetScreenPosition(const wxPoint2DDouble& shapePos, double width, double height, double scale, const wxPoint2DDouble& offset) const;

    /** @brief 포트를 화면에 원 형태로 그리기 */
    void Draw(wxDC& dc, const wxPoint& screenPos) const;
};
