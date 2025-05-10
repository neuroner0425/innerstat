#pragma once
#include <wx/wx.h>
#include <string>

class Port
{
public:
    std::string id;
    wxPoint2DDouble relativePos; // 도형 내부에서의 상대 위치 (0.0 ~ 1.0)

    mutable wxPoint cachedScreenPos; // 화면상의 절대 좌표 (렌더링용)

    Port(const std::string &id, const wxPoint2DDouble &relPos);

    wxPoint GetScreenPosition(const wxPoint2DDouble &shapePos, double width, double height, double scale, const wxPoint2DDouble &offset) const; // 도형 정보 기준으로 포트의 화면 좌표를 계산하고 캐싱함
    void Draw(wxDC &dc, const wxPoint &screenPos) const; // 포트를 화면에 그림
};
