#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cwchar>
#include <windows.h>

// ===== Console colour and pixel definitions =====
enum COLOUR {
    FG_BLACK        = 0x0000,
    FG_DARK_BLUE    = 0x0001,
    FG_DARK_GREEN   = 0x0002,
    FG_DARK_CYAN    = 0x0003,
    FG_DARK_RED     = 0x0004,
    FG_DARK_MAGENTA = 0x0005,
    FG_DARK_YELLOW  = 0x0006,
    FG_GREY         = 0x0007,
    FG_DARK_GREY    = 0x0008,
    FG_BLUE         = 0x0009,
    FG_GREEN        = 0x000A,
    FG_CYAN         = 0x000B,
    FG_RED          = 0x000C,
    FG_MAGENTA      = 0x000D,
    FG_YELLOW       = 0x000E,
    FG_WHITE        = 0x000F,
};
const wchar_t PIXEL_SOLID = 0x2588;

// ===== Console Game Engine (threadless version) =====
class olcConsoleGameEngine {
public:
    olcConsoleGameEngine() {
        m_nScreenWidth = 80;
        m_nScreenHeight = 30;
        m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
        m_bAtomActive = true;
    }
    virtual ~olcConsoleGameEngine() {}

    int ScreenWidth() { return m_nScreenWidth; }
    int ScreenHeight() { return m_nScreenHeight; }

    void Draw(int x, int y, wchar_t c = PIXEL_SOLID, short col = FG_WHITE) {
        if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight) {
            m_bufScreen[y * m_nScreenWidth + x].Char.UnicodeChar = c;
            m_bufScreen[y * m_nScreenWidth + x].Attributes = col;
        }
    }

    void Fill(int x1, int y1, int x2, int y2, wchar_t c = PIXEL_SOLID, short col = FG_WHITE) {
        Clip(x1, y1);
        Clip(x2, y2);
        for (int x = x1; x < x2; x++)
            for (int y = y1; y < y2; y++)
                Draw(x, y, c, col);
    }

    void DrawString(int x, int y, std::wstring c, short col = FG_WHITE) {
        for (size_t i = 0; i < c.size(); i++) {
            if (x + i >= 0 && x + i < m_nScreenWidth && y >= 0 && y < m_nScreenHeight) {
                m_bufScreen[y * m_nScreenWidth + x + i].Char.UnicodeChar = c[i];
                m_bufScreen[y * m_nScreenWidth + x + i].Attributes = col;
            }
        }
    }

    void Clip(int &x, int &y) {
        if (x < 0) x = 0;
        if (x >= m_nScreenWidth) x = m_nScreenWidth;
        if (y < 0) y = 0;
        if (y >= m_nScreenHeight) y = m_nScreenHeight;
    }

    // Threadless game loop
    void Start() {
        m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
        memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);
        m_bAtomActive = true;

        if (!OnUserCreate())
            m_bAtomActive = false;

        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = tp1;

        while (m_bAtomActive) {
            tp2 = std::chrono::system_clock::now();
            std::chrono::duration<float> elapsedTime = tp2 - tp1;
            tp1 = tp2;
            float fElapsedTime = elapsedTime.count();

            if (!OnUserUpdate(fElapsedTime))
                m_bAtomActive = false;

            WriteConsoleOutput(m_hConsole, m_bufScreen,
                { (short)m_nScreenWidth, (short)m_nScreenHeight },
                { 0, 0 }, &m_rectWindow);
        }
    }

 void ConstructConsole(int width, int height, int fontw, int fonth)
{
    m_nScreenWidth = width;
    m_nScreenHeight = height;

    // Create screen buffer
    m_bufScreen = new CHAR_INFO[m_nScreenWidth * m_nScreenHeight];
    memset(m_bufScreen, 0, sizeof(CHAR_INFO) * m_nScreenWidth * m_nScreenHeight);

    // Set console size
    m_rectWindow = { 0, 0, 1, 1 };
    SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

    COORD coord = { (short)m_nScreenWidth, (short)m_nScreenHeight };
    SetConsoleScreenBufferSize(m_hConsole, coord);
    SetConsoleActiveScreenBuffer(m_hConsole);

    // Just set the visible window size (no font adjustments)
    m_rectWindow = { 0, 0, (short)(m_nScreenWidth - 1), (short)(m_nScreenHeight - 1) };
    SetConsoleWindowInfo(m_hConsole, TRUE, &m_rectWindow);

    m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
}


protected:
    virtual bool OnUserCreate() = 0;
    virtual bool OnUserUpdate(float fElapsedTime) = 0;

protected:
    int m_nScreenWidth;
    int m_nScreenHeight;
    CHAR_INFO* m_bufScreen;
    HANDLE m_hConsole;
    HANDLE m_hConsoleIn;
    SMALL_RECT m_rectWindow = { 0, 0, 1, 1 };
    bool m_bAtomActive;
};

// ===== Flappy Bird Game =====
class OneLoneCoder_FlappyBird : public olcConsoleGameEngine {
public:
    OneLoneCoder_FlappyBird() { m_sAppName = L"Flappy Bird"; }

private:
    std::wstring m_sAppName;
    float fBirdPosition = 0.0f;
    float fBirdVelocity = 0.0f;
    float fBirdAcceleration = 0.0f;
    float fGravity = 100.0f;
    float fLevelPosition = 0.0f;
    float fSectionWidth;
    std::list<int> listSection;
    bool bHasCollided = false;
    bool bResetGame = false;
    int nAttemptCount = 0;
    int nFlapCount = 0;
    int nMaxFlapCount = 0;

protected:
    bool OnUserCreate() override {
        listSection = { 0, 0, 0, 0 };
        bResetGame = true;
        fSectionWidth = (float)ScreenWidth() / (float)(listSection.size() - 1);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
        if (bResetGame) {
            bHasCollided = false;
            bResetGame = false;
            listSection = { 0, 0, 0, 0 };
            fBirdAcceleration = 0.0f;
            fBirdVelocity = 0.0f;
            fBirdPosition = ScreenHeight() / 2.0f;
            nFlapCount = 0;
            nAttemptCount++;
        }

        if (bHasCollided) {
            DrawString(ScreenWidth()/2 - 5, ScreenHeight()/2, L"GAME OVER", FG_RED);
            if (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
                bResetGame = true;
        } else {
            if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && fBirdVelocity >= fGravity / 10.0f) {
                fBirdAcceleration = 0.0f;
                fBirdVelocity = -fGravity / 4.0f;
                nFlapCount++;
                if (nFlapCount > nMaxFlapCount) nMaxFlapCount = nFlapCount;
            } else {
                fBirdAcceleration += fGravity * fElapsedTime;
            }

            if (fBirdAcceleration >= fGravity) fBirdAcceleration = fGravity;
            fBirdVelocity += fBirdAcceleration * fElapsedTime;
            fBirdPosition += fBirdVelocity * fElapsedTime;
            fLevelPosition += 14.0f * fElapsedTime;

            if (fLevelPosition > fSectionWidth) {
                fLevelPosition -= fSectionWidth;
                listSection.pop_front();
                int i = rand() % (ScreenHeight() - 20);
                if (i <= 10) i = 0;
                listSection.push_back(i);
            }

            Fill(0, 0, ScreenWidth(), ScreenHeight(), L' ');

            int nSection = 0;
            for (auto s : listSection) {
                if (s != 0) {
                    Fill(nSection * fSectionWidth + 10 - fLevelPosition, ScreenHeight() - s, nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight(), PIXEL_SOLID, FG_GREEN);
                    Fill(nSection * fSectionWidth + 10 - fLevelPosition, 0, nSection * fSectionWidth + 15 - fLevelPosition, ScreenHeight() - s - 15, PIXEL_SOLID, FG_GREEN);
                }
                nSection++;
            }

            int nBirdX = (int)(ScreenWidth() / 3.0f);

            bHasCollided = fBirdPosition < 2 || fBirdPosition > ScreenHeight() - 2 ||
                m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX].Char.UnicodeChar != L' ' ||
                m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX].Char.UnicodeChar != L' ' ||
                m_bufScreen[(int)(fBirdPosition + 0) * ScreenWidth() + nBirdX + 6].Char.UnicodeChar != L' ' ||
                m_bufScreen[(int)(fBirdPosition + 1) * ScreenWidth() + nBirdX + 6].Char.UnicodeChar != L' ';

            // Yellow Bird
            if (fBirdVelocity > 0) {
                DrawString(nBirdX, fBirdPosition + 0, L" \\\\ ", FG_YELLOW);
                DrawString(nBirdX, fBirdPosition + 1, L"<\\\\=Q", FG_YELLOW);
            } else {
                DrawString(nBirdX, fBirdPosition + 0, L"<//=Q", FG_YELLOW);
                DrawString(nBirdX, fBirdPosition + 1, L" // ", FG_YELLOW);
            }

            std::wstringstream ss;
            ss << L"Attempt: " << nAttemptCount << L"  Max Flaps: " << nMaxFlapCount;
            DrawString(1, 1, ss.str());
        }
        return true;
    }
};

int main() {
    OneLoneCoder_FlappyBird game;
    game.ConstructConsole(80, 48, 16, 16);
    game.Start();
    return 0;
}
