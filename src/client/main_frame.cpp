#ifndef INNERSTAT_CLIENT_BASE_H
    #include "innerstat/client/client.h"
#endif

#include <wx/wx.h>
#include "innerstat/client/main_frame.h"
#include "innerstat/client/canvas.h"
#include "innerstat/client/agent_mqtt_connection.h"
#include "innerstat/client/AgentSelectionDialog.h"
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <regex>
#include <wx/srchctrl.h>
#include <wxbf/borderless_frame.h>
#include <wxbf/system_buttons.h>
#include <wxbf/window_gripper.h>
#ifdef __WXOSX__
#include <wxbf/borderless_frame_osx.h>
#endif

#include "innerstat/client/canvas.h"
#include "innerstat/client/dialog.h"
#include "innerstat/client/color_manager.h"
#include "innerstat/client/main_frame.h"
#include "innerstat/client/agent_dialog.h"
#include "innerstat/client/AgentSelectionDialog.h"
#include "no_sash_splitter.h"

#include "innerstat/client/shape.h"
#include <map>

INNERSTAT_BEGIN_NAMESPACE

// MainFrame의 keyPressed 배열 정의 (400개, 모두 false로 초기화)
bool MainFrame::keyPressed[400] = { false };

MainFrame::MainFrame()
    :wxBorderlessFrame(nullptr, wxID_ANY, L"Inner Stat", wxDefaultPosition, wxSize(1100, 700)), gripper(wxWindowGripper::Create()){
    
    SetUpGUI();
    
    Bind(wxEVT_PAINT, &MainFrame::OnPaint, this);
    Bind(wxEVT_SIZE, &MainFrame::OnSize, this);

    addAreaBtn->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
        addTopLevelArea();
    });
    shapeTree->Bind(wxEVT_TREE_SEL_CHANGED, [=](wxTreeEvent &evt) {
        canvas->OnTreeSelectionChanged(evt.GetItem());
    });
    shapeTree->Bind(wxEVT_TREE_ITEM_ACTIVATED, [=](wxTreeEvent &evt) {
        canvas->OnTreeLeftDClick(evt.GetItem());
    });

    // Setup and start the request timer
    m_request_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &MainFrame::OnRequestTimer, this, m_request_timer.GetId());
    m_request_timer.Start(5000);

    // Setup and start the status update timer
    m_update_timer.SetOwner(this);
    Bind(wxEVT_TIMER, &MainFrame::OnUpdateTimer, this, m_update_timer.GetId());
    m_update_timer.Start(1000); // Update status every second

    wxSizeEvent dummy;
    OnSize(dummy);
}

void MainFrame::addTopLevelArea() {
    if (agent_selection_dialog_) {
        agent_selection_dialog_->Raise();
        agent_selection_dialog_->SetFocus();
    } else {
        agent_selection_dialog_ = new AgentSelectionDialog(this, canvas);
        agent_selection_dialog_->Show();
        // Register a handler to nullify the pointer when the dialog is closed
        agent_selection_dialog_->Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event) {
            agent_selection_dialog_ = nullptr;
            event.Skip(); // Allow the dialog's own handler to run and destroy it
        });
    }
}

void MainFrame::OnUpdateTimer(wxTimerEvent& event) {
    if (!canvas) return;

    auto mqtt_conn = AgentMqttConnection::GetInstance();
    const auto& all_agent_data = mqtt_conn->GetAgentDataMap();
    const auto& all_log_timestamps = mqtt_conn->GetLogTimestamps();
    const auto& all_shapes = canvas->GetAllShapes();
    long long now = wxGetUTCTimeMillis().GetValue();

    bool needs_refresh = false;

    for (Shape* shape : all_shapes) {
        ShapeStatus old_status = shape->status;

        if (shape->type == ShapeType::OS) {
            auto it = all_agent_data.find(shape->mac_address);
            if (it == all_agent_data.end()) {
                // No info ever received for this MAC
                if ((now - shape->last_seen_timestamp.GetValue()) > (shape->loss_threshold_seconds * 1000)) {
                    shape->status = ShapeStatus::Lost;
                }
            } else {
                // We have data for this MAC, check how old it is
                const systemInfo& info = it->second.first;
                const wxLongLong& last_message_timestamp = it->second.second;
                long long last_msg_time = last_message_timestamp.GetValue();
                long long threshold = shape->loss_threshold_seconds * 1000;

                if ((now - last_msg_time) > threshold) {
                    shape->status = ShapeStatus::Lost;
                } else {
                    // It's not lost, check other statuses
                    if (info.status.cpu_usage > shape->warning_threshold_cpu) {
                        shape->status = ShapeStatus::Warning;
                    } else if (info.status.cpu_usage > shape->attention_threshold_cpu) {
                        shape->status = ShapeStatus::Attention;
                    } else {
                        shape->status = ShapeStatus::Normal;
                    }
                }
            }
        } else if (shape->type == ShapeType::PS) {
            if (!shape->parent || shape->parent->type != ShapeType::OS) {
                continue; 
            }

            // If parent is Lost, child is also Lost.
            if (shape->parent->status == ShapeStatus::Lost) {
                shape->status = ShapeStatus::Lost;
            } else {
                // Parent is NOT Lost. Check for this PS shape's own status.
                auto agent_it = all_agent_data.find(shape->parent->mac_address);
                if (agent_it == all_agent_data.end()) {
                    // This case should be covered by parent status check, but as a safeguard:
                    if ((now - shape->last_seen_timestamp.GetValue()) > (shape->loss_threshold_seconds_ps * 1000)) {
                        shape->status = ShapeStatus::Lost;
                    }
                    continue;
                }

                // Check if the port is in the lsof list from the recent parent data
                const auto& lsof_items = agent_it->second.first.lsof_items;
                bool found_in_lsof = false;
                try {
                    std::regex port_regex(":" + std::to_string(shape->port_number) + "([^0-9]|$)");
                    for (const auto& item : lsof_items) {
                        if (std::regex_search(item.name, port_regex)) {
                            found_in_lsof = true;
                            break;
                        }
                    }
                } catch (const std::regex_error& e) {
                    // In case of a bad regex, just fallback to old behavior
                }
                
                if (found_in_lsof) {
                    // Port is active. Update timestamp and check log thresholds.
                    shape->last_seen_timestamp = now;
                    
                    int log_count = 0;
                    auto log_mac_it = all_log_timestamps.find(shape->parent->mac_address);
                    if (log_mac_it != all_log_timestamps.end()) {
                        auto log_port_it = log_mac_it->second.find(shape->port_number);
                        if (log_port_it != log_mac_it->second.end()) {
                            const auto& timestamps = log_port_it->second;
                            for (auto rit = timestamps.rbegin(); rit != timestamps.rend(); ++rit) {
                                if ((now - rit->GetValue()) < 60000) { // 60 seconds
                                    log_count++;
                                } else {
                                    break; // Timestamps are ordered
                                }
                            }
                        }
                    }

                    if (log_count > shape->warning_threshold_log) {
                        shape->status = ShapeStatus::Warning;
                    } else if (log_count > shape->attention_threshold_log) {
                        shape->status = ShapeStatus::Attention;
                    } else {
                        shape->status = ShapeStatus::Normal;
                    }
                } else {
                    // Port is NOT in the lsof list, even though parent is alive.
                    // Check how long it has been missing.
                    if ((now - shape->last_seen_timestamp.GetValue()) > (shape->loss_threshold_seconds_ps * 1000)) {
                        shape->status = ShapeStatus::Lost;
                    }
                    // If missing for less than the threshold, retain previous status.
                }
            }
        }

        if (old_status != shape->status) {
            needs_refresh = true;
        }
    }

    if (needs_refresh) {
        canvas->Refresh();
    }
}

void MainFrame::OnRequestTimer(wxTimerEvent& event) {
    if (!canvas) return;

    const auto& allShapes = canvas->GetAllShapes();
    std::map<std::string, std::vector<int>> requests; // Map MAC -> list of ports

    // First pass: collect all ports for each OS shape
    for (Shape* shape : allShapes) {
        if (shape->type == ShapeType::PS && shape->parent && shape->parent->type == ShapeType::OS) {
            if (!shape->parent->mac_address.empty() && shape->port_number > 0) {
                requests[shape->parent->mac_address].push_back(shape->port_number);
            }
        }
    }

    // Second pass: send the requests
    auto mqtt_conn = AgentMqttConnection::GetInstance();
    for (const auto& pair : requests) {
        const std::string& mac = pair.first;
        const std::vector<int>& ports = pair.second;

        // Construct JSON: [port1, port2, ...]
        std::string payload = "[";
        for (size_t i = 0; i < ports.size(); ++i) {
            payload += std::to_string(ports[i]);
            if (i < ports.size() - 1) {
                payload += ",";
            }
        }
        payload += "]";

        std::string topic = "innerstat/" + mac + "/req";
        mqtt_conn->publish(NULL, topic.c_str(), payload.length(), payload.c_str());
    }
}

void MainFrame::SetUpGUI(){
    this->SetBorderColour(C_TITLE_BAR_BACKGROUND);

#ifndef __WXOSX__
    systemButtons = wxSystemButtonsFactory::CreateSystemButtons(this);
    // On Windows/Linux, use larger rectangular buttons matching the app style
    systemButtons->SetButtonSize(wxSize(46, 35));
    systemButtons->UpdateState();
    SetSystemButtonColor();
    titlebarHeight = systemButtons->GetTotalSize().y;
#else
    // macOS: use native traffic lights, no custom-drawn buttons
    systemButtons = nullptr;
    titlebarHeight = FromDIP(35);
#endif

    menuBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, titlebarHeight));
    menuBar->SetBackgroundColour(C_TITLE_BAR_BACKGROUND);

    wxBoxSizer* menuSizer = new wxBoxSizer(wxHORIZONTAL);

#ifndef __WXOSX__
    wxButton* btnFile = makeMenuBtn(L"파일(F)", {L"새 파일", L"열기", L"저장", L"다른 이름으로 저장"});
    wxButton* btnEdit = makeMenuBtn(L"편집(E)", {L"실행 취소", L"다시 실행", L"복사", L"붙여넣기"});
    wxButton* btnSelect = makeMenuBtn(L"선택 영역(S)", {L"모두 선택", L"줄 선택", L"블록 선택"});

    menuSizer->Add(btnFile,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnEdit,   0, wxLEFT | wxALIGN_CENTER_VERTICAL);
    menuSizer->Add(btnSelect, 0, wxLEFT | wxALIGN_CENTER_VERTICAL);
#elif __WXOSX__
    wxMenu *fileMenu = new wxMenu();
    fileMenu->Append(wxID_OPEN, L"열기(&O)");
    fileMenu->Append(wxID_SAVE, L"저장(&S)");

    wxMenu *editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, L"실행 취소(&Z)");
    editMenu->Append(wxID_REDO, L"다시 실행(&Y)");
    editMenu->Append(wxID_COPY, L"복사(&C)");
    editMenu->Append(wxID_PASTE, L"붙여넣기(&V)");

    wxMenu *selectMenu = new wxMenu();
    selectMenu->Append(wxID_SELECTALL, L"모두 선택(&A)");
    selectMenu->Append(wxID_ANY, L"줄 선택");
    selectMenu->Append(wxID_ANY, L"블록 선택");

    wxMenuBar *menu = new wxMenuBar();
    menu->Append(fileMenu, L"파일");
    menu->Append(editMenu, L"편집");
    menu->Append(selectMenu, L"선택 영역");
    SetMenuBar(menu);
#endif
    menuBar->SetSizer(menuSizer);

#ifdef __WXOSX__
    // Install menuBar as a native titlebar accessory alongside traffic lights
    if (auto osx = dynamic_cast<wxBorderlessFrameOSX*>(this)) {
        osx->EnableUnifiedTitlebar(true);
        osx->SetMovableByBackground(false);
        osx->SetTitlebarAccessory(menuBar, 1, 1);
    }
    // Drag logic is handled natively by the titlebar visual effect view; no app-level hacks needed
#endif

    leftToolbar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(leftbarWidth, 100));
    leftToolbar->SetBackgroundColour(C_TOOL_BAR_BACKGROUND);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    wxButton* btn1 = makeToolBtn("F");
    btn1->Bind(wxEVT_BUTTON, [=](wxCommandEvent &) {
        AgentComDialog* dlg = new AgentComDialog(this);
        dlg->Show();
    });
    wxButton* btn2 = makeToolBtn("S");
    wxButton* btn3 = makeToolBtn("G");

    leftSizer->AddSpacer(10);
    leftSizer->Add(btn1, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->Add(btn3, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 4);
    leftSizer->AddStretchSpacer();
    leftToolbar->SetSizer(leftSizer);

    mainContent = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(700, 600));
    mainContent->SetBackgroundColour(C_CLIENT_BACKGROUND);

    wxSplitterWindow *splitter = new wxNoSashSplitterWindow(mainContent);
    wxPanel *leftPanel = new wxPanel(splitter);
    wxBoxSizer *treeViewSizer = new wxBoxSizer(wxVERTICAL);
    shapeTree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HAS_BUTTONS);
    shapeTree->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    SetWindowColor(shapeTree, C_TREE_VIEW_FOREGROUND, C_TREE_VIEW_BACKGROUND);
    addAreaBtn = new wxButton(leftPanel, wxID_ANY, "Add Shape");
    addAreaBtn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    SetWindowColor(addAreaBtn, C_TREE_VIEW_FOREGROUND, C_TREE_VIEW_BACKGROUND, C_TREE_VIEW_FOREGROUND, C_TITLE_BAR_HOVER_BACKGROUND);

    treeViewSizer->Add(addAreaBtn, 0, wxEXPAND | wxBOTTOM, 3);
    treeViewSizer->Add(shapeTree, 1, wxEXPAND | wxBOTTOM, 5);
    leftPanel->SetSizer(treeViewSizer);

    canvas = new MainCanvas(splitter, this);
    canvas->SetBackgroundColour(C_CLIENT_BACKGROUND);

    splitter->SetMinimumPaneSize(120);
    splitter->SetBackgroundColour(C_TREE_VIEW_BACKGROUND);
    splitter->Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&){});

    splitter->SplitVertically(leftPanel, canvas);

    CallAfter([=]() {
        splitter->SetSashPosition(350);
        splitter->Refresh();
        splitter->Update();
    });

    wxBoxSizer* mainContentSizer = new wxBoxSizer(wxVERTICAL);
    mainContentSizer->Add(splitter, 1, wxEXPAND);
    mainContent->SetSizer(mainContentSizer);

    mainContent->Layout();
    this->SetMinClientSize(wxSize(350, 200));

    Refresh();
}

void MainFrame::SetSystemButtonColor(){
    auto setButtonColor = [this](int button, int state, int kind, const wxColour& color) {
        for (size_t buttonKind = (button < 0 ? 0 : button); buttonKind <= (button < 0 ? 3 : button); ++buttonKind) {
            for (size_t stateKind = (state < 0 ? 0 : state); stateKind <= (state < 0 ? 4 : state); ++stateKind) {
                systemButtons->SetColourTableEntry(
                    static_cast<wxSystemButton>(buttonKind),
                    static_cast<wxSystemButtonState>(stateKind),
                    static_cast<wxSystemButtonColourKind>(kind),
                    color);
            }
        }
    };

    setButtonColor(-1, -1, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_BACKGROUND);
    setButtonColor(-1, -1, wxSystemButtonColourKind::wxSB_COLOUR_FOREGROUND, C_TITLE_BAR_FOREGROUND);

    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_HOVER, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_HOVER_BACKGROUND);
    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_PRESSED, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_PRESSED_BACKGROUND);

    
    setButtonColor(wxSystemButton::wxSB_CLOSE, wxSystemButtonState::wxSB_STATE_HOVER, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_CLOSE_HOVER_BACKGROUND);
    setButtonColor(wxSystemButton::wxSB_CLOSE, wxSystemButtonState::wxSB_STATE_PRESSED, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_CLOSE_PRESSED_BACKGROUND);

    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_INACTIVE, wxSystemButtonColourKind::wxSB_COLOUR_BACKGROUND, C_TITLE_BAR_INACTIVE_BACKGROUND);
    setButtonColor(-1, wxSystemButtonState::wxSB_STATE_INACTIVE, wxSystemButtonColourKind::wxSB_COLOUR_FOREGROUND, C_TITLE_BAR_INACTIVE_FOREGROUND);
    
    systemButtons->UpdateState();
}

void MainFrame::OnPaint(wxPaintEvent& event)
{
#ifdef __WXOSX__
    // Native titlebar (NSVisualEffectView) provides its own background; avoid custom paint overlay
    event.Skip();
    return;
#endif
    wxPaintDC dc(this);
    dc.SetBrush(C_TITLE_BAR_BACKGROUND);
    dc.SetPen(C_TITLE_BAR_BACKGROUND);
    const int w = GetClientSize().x;
    dc.DrawRectangle(0, 0, w, titlebarHeight);

    event.Skip();
}


void MainFrame::OnSize(wxSizeEvent& event)
{
    wxSize sz = GetClientSize();

    // Position menu bar to the right of left-aligned system buttons (macOS)
    int leftInset = 0;
    if (systemButtons && !systemButtons->AreButtonsRightAligned()) {
        leftInset = systemButtons->GetTotalSize().x + FromDIP(8);
    }
#ifndef __WXOSX__
    // Windows/Linux: menuBar가 전체 가로를 차지하도록
    menuBar->SetSize(leftInset, 0, sz.x - leftInset, titlebarHeight);
#else
    // macOS: titlebar accessory도 창 크기에 맞게 width 갱신
    // menuBar->SetSize(0, 0, sz.x, titlebarHeight);
#endif

    int searchW = 320, searchH = 30;

    leftToolbar->SetSize(0, titlebarHeight, leftbarWidth, sz.y - titlebarHeight);

    mainContent->SetSize(leftbarWidth, titlebarHeight, sz.x - leftbarWidth, sz.y - titlebarHeight);

    event.Skip();
}

wxButton* MainFrame::makeMenuBtn(const wxString& label, const wxArrayString subItems) {
    int textWidth, textHeight;
    int padding = 20;
    wxButton* btn = new wxButton(menuBar, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    SetWindowColor(btn, C_TITLE_BAR_FOREGROUND, C_TITLE_BAR_BACKGROUND, C_TITLE_BAR_FOREGROUND, C_TITLE_BAR_HOVER_BACKGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);

    menuBar->GetTextExtent(label, &textWidth, &textHeight);
    btn->SetMinSize(wxSize(textWidth + padding, -1));

    btn->Bind(wxEVT_BUTTON, [this, btn, subItems](wxCommandEvent&) {
        wxMenu popup;
        for(const auto& text : subItems) {
            popup.Append(wxID_ANY, text);
        }
        wxPoint btnScreen = btn->ClientToScreen(wxPoint(0, btn->GetSize().y));
        PopupMenu(&popup, ScreenToClient(btnScreen));
    });

    return btn;
};


wxButton* MainFrame::makeToolBtn(const wxString& label) {
    wxButton* btn = new wxButton(leftToolbar, wxID_ANY, label, wxDefaultPosition, wxSize(40, 40));
    SetWindowColor(btn, C_TOOL_BAR_FOREGROUND, C_TOOL_BAR_BACKGROUND, C_TOOL_BAR_HOVER_FOREGROUND, C_TOOL_BAR_BACKGROUND);
    btn->SetWindowStyleFlag(wxBORDER_NONE | wxTRANSPARENT_WINDOW);
    btn->SetCursor(wxCursor(wxCURSOR_HAND));
    return btn;
};

void MainFrame::SetWindowColor(wxWindow* windows, const wxColour color_default_fore, const wxColour color_default_back, 
        const wxColour color_hover_fore, const wxColour color_hover_back){
    windows->SetForegroundColour(color_default_fore);
    windows->SetBackgroundColour(color_default_back);
    
    if(color_hover_fore != wxColor(1,2,3)){
        windows->Bind(wxEVT_ENTER_WINDOW, [=](wxMouseEvent&) {
            windows->SetForegroundColour(color_hover_fore);
            windows->SetBackgroundColour(color_hover_back);
            windows->Refresh();
        });
        windows->Bind(wxEVT_LEAVE_WINDOW, [=](wxMouseEvent&) {
            windows->SetForegroundColour(color_default_fore);
            windows->SetBackgroundColour(color_default_back);
            windows->Refresh();
        });
    }
}

INNERSTAT_END_NAMESPACE