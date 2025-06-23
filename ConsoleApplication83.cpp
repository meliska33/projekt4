#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>
#include <map>
#include <iostream>
#include <set>
#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken;

const int floorCount = 5;
const int floorHeight = 100;
const int cabinHeight = 95;
const int baseY = 40;
const int maxCapacity = 8;
const int elevatorSpeed = 6;
int currentFloor = 0;
int targetFloor = 0;
int elevatorY = baseY + (floorCount - 1 - currentFloor) * floorHeight;
int elevatorDirection = 0;

struct Person {
    int startFloor;
    int targetFloor;
    bool inElevator = false;
};

#define ID_TIMER 1
#define ID_IDLE_TIMER 2

int floorButtons[floorCount] = { 0, 1, 2, 3, 4 };
HWND hButtons[floorCount];
std::vector<Person> people = {};
std::set<int> pickupFloors;
std::set<int> dropoffFloors;

std::chrono::steady_clock::time_point lastActivityTime = std::chrono::steady_clock::now();

std::map<int, Color> floorColors = {
    {0, Color(255, 255, 200, 150)},
    {1, Color(255, 255, 100, 100)},
    {2, Color(255, 100, 255, 100)},
    {3, Color(255, 100, 100, 255)},
    {4, Color(255, 255, 100, 255)}
};

int countPassengersInElevator() {
    int count = 0;
    for (const auto& p : people) {
        if (p.inElevator) count++;
    }
    return count * 70;
}

void addPickupFloor(int floor) {
    pickupFloors.insert(floor);
    lastActivityTime = std::chrono::steady_clock::now();
}

void addDropoffFloor(int floor) {
    dropoffFloors.insert(floor);
    lastActivityTime = std::chrono::steady_clock::now();
}

void updateTargetFloor() {
    if (!dropoffFloors.empty()) {
        int minDist = floorCount + 1;
        int selectedFloor = currentFloor;
        for (int f : dropoffFloors) {
            int dist = abs(f - currentFloor);
            if (dist < minDist) {
                minDist = dist;
                selectedFloor = f;
            }
        }

        targetFloor = selectedFloor;
        elevatorDirection = (targetFloor > currentFloor) ? 1 : -1;
        return;
    }

    if (!pickupFloors.empty()) {
        int minDist = floorCount + 1;
        int selectedFloor = currentFloor;
        for (int f : pickupFloors) {
            int dist = abs(f - currentFloor);
            if (dist < minDist) {
                minDist = dist;
                selectedFloor = f;
            }
        }

        targetFloor = selectedFloor;
        elevatorDirection = (targetFloor > currentFloor) ? 1 : -1;
        return;
    }

    elevatorDirection = 0;
}

void checkIdleState() {
    auto now = std::chrono::steady_clock::now();

    if (currentFloor != 0 && dropoffFloors.empty() && pickupFloors.empty() && elevatorDirection == 0) {
        auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastActivityTime).count();
        if (idleDuration >= 5) {
            targetFloor = 0;
            elevatorDirection = -1;
            lastActivityTime = std::chrono::steady_clock::now();
        }
    }
}

void moveElevatorStep(HWND hwnd) {
    static bool loading = false;
    static int loadingTimer = 0;

    int targetY = baseY + (floorCount - 1 - targetFloor) * floorHeight;

    if (!loading) {
        if (elevatorY < targetY)
            elevatorY += elevatorSpeed;
        else if (elevatorY > targetY)
            elevatorY -= elevatorSpeed;

        if (abs(elevatorY - targetY) < elevatorSpeed) {
            elevatorY = targetY;
            currentFloor = targetFloor;
            loading = true;
            loadingTimer = 0;
        }
    }
    else {
        loadingTimer++;
        if (loadingTimer > 15) {
            loadingTimer = 0;
            loading = false;

            for (size_t i = 0; i < people.size();) {
                if (people[i].inElevator && people[i].targetFloor == currentFloor) {
                    people.erase(people.begin() + i);
                    dropoffFloors.erase(currentFloor);
                    lastActivityTime = std::chrono::steady_clock::now();
                }
                else {
                    i++;
                }
            }

            int currentPassengers = countPassengersInElevator();
            if (currentPassengers < maxCapacity) {
                for (auto& p : people) {
                    if (!p.inElevator && p.startFloor == currentFloor && currentPassengers < maxCapacity) {
                        p.inElevator = true;
                        addDropoffFloor(p.targetFloor);
                        currentPassengers++;
                        lastActivityTime = std::chrono::steady_clock::now();
                    }
                }
            }

            bool stillWaiting = false;
            for (const auto& p : people) {
                if (!p.inElevator && p.startFloor == currentFloor) {
                    stillWaiting = true;
                    break;
                }
            }
            if (!stillWaiting) {
                pickupFloors.erase(currentFloor);
            }

            updateTargetFloor();
        }
    }

    checkIdleState();
    InvalidateRect(hwnd, NULL, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        SetTimer(hwnd, ID_TIMER, 16, NULL);
        SetTimer(hwnd, ID_IDLE_TIMER, 1000, NULL);

        for (int i = 0; i < floorCount; ++i) {
            wchar_t label[2] = { L'0' + (wchar_t)i, 0 };
            hButtons[i] = CreateWindow(L"BUTTON", label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20 + i * 40, 20, 35, 25,
                hwnd, (HMENU)floorButtons[i], NULL, NULL);
        }

        for (int floor = 0; floor < floorCount; ++floor) {
            for (int dest = 0; dest < floorCount; ++dest) {
                wchar_t label[3] = { L'0' + dest, 0 };
                CreateWindow(
                    L"BUTTON", label,
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    20 + dest * 35,
                    baseY + (floorCount - 1 - floor) * floorHeight + 70,
                    30, 25,
                    hwnd,
                    (HMENU)(1000 + floor * floorCount + dest),
                    NULL, NULL);
            }
        }
        return 0;
    }

    case WM_COMMAND:
        for (int i = 0; i < floorCount; ++i) {
            if (LOWORD(wParam) == floorButtons[i]) {
                addPickupFloor(i);
                if (elevatorDirection == 0) {
                    updateTargetFloor();
                }
                break;
            }
        }

        {
            int id = LOWORD(wParam);
            if (id >= 1000 && id < 1000 + floorCount * floorCount) {
                int localId = id - 1000;
                int floor = localId / floorCount;
                int target = localId % floorCount;

                if (floor != target) {
                    Person p = { floor, target, false };
                    people.push_back(p);
                    addPickupFloor(floor);
                    if (elevatorDirection == 0) {
                        updateTargetFloor();
                    }
                }
            }
        }
        return 0;

    case WM_TIMER:
        if (wParam == ID_TIMER) {
            moveElevatorStep(hwnd);
        }
        else if (wParam == ID_IDLE_TIMER) {
            checkIdleState();
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        Graphics g(memDC);
        g.Clear(Color(255, 230, 230, 230));

        SolidBrush shaftBrush(Color(255, 100, 100, 100));
        g.FillRectangle(&shaftBrush, 270, baseY, 100, floorCount * floorHeight);

        Pen floorPen(Color(255, 80, 80, 80), 1);
        for (int i = 1; i < floorCount + 1; ++i) {
            int y = baseY + i * floorHeight;
            g.DrawLine(&floorPen, 170, y, 470, y);
        }

        SolidBrush cabinBrush(Color(255, 180, 180, 180));
        g.FillRectangle(&cabinBrush, 280, elevatorY, 80, cabinHeight);

        const int spacing = 25;

        for (int floor = 0; floor < floorCount; ++floor) {
            int y = baseY + (floorCount - 1 - floor) * floorHeight + 30;
            int drawn = 0;

            for (const auto& person : people) {
                if (!person.inElevator && person.startFloor == floor) {
                    int x = 180 - drawn * spacing;
                    SolidBrush personBrush(floorColors[person.targetFloor]);
                    g.FillEllipse(&personBrush, x, y, 20, 20);
                    drawn++;
                }
            }
        }

        int Y = elevatorY + 10;
        int X = 285;
        for (const auto& person : people) {
            if (person.inElevator) {
                SolidBrush personBrush(floorColors[person.targetFloor]);
                g.FillEllipse(&personBrush, X, Y, 20, 20);
                X += 25;
                if (X > 350) {
                    X = 285;
                    Y += 25;
                }
            }
        }

        WCHAR floorText[64];
        swprintf_s(floorText, L"Pietro: %d | Udzwig: %d kg /%d kg",
            currentFloor, countPassengersInElevator(), maxCapacity * 75);
        SolidBrush textBrush(Color(255, 0, 0, 0));
        Font font(L"Arial", 12);
        g.DrawString(floorText, -1, &font, PointF(10, 5), &textBrush);

        int legendY = baseY + floorCount * floorHeight + 20;
        for (const auto& fc : floorColors) {
            WCHAR legendText[32];
            swprintf_s(legendText, L"Pietro %d:", fc.first);
            g.DrawString(legendText, -1, &font, PointF(10, legendY), &textBrush);

            SolidBrush legendBrush(fc.second);
            g.FillEllipse(&legendBrush, 80, legendY, 15, 15);

            legendY += 20;
        }

        BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER);
        KillTimer(hwnd, ID_IDLE_TIMER);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"The Winda";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"The Winda",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 700,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}