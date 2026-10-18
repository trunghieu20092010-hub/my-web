// =====================================================================
//  Chúc mừng 20/10 - Gửi các bạn nữ lớp 11A1
//  Hoạt ảnh trên terminal bằng C++17 (màu 24-bit ANSI, chạy Windows/Linux/macOS)
//  Biên dịch:  g++ -std=c++17 -O2 main.cpp -o chuc2010
//  Chạy thử nhanh: ./chuc2010 --fast      Tắt tiếng: ./chuc2010 --mute
//  Xuất file âm thanh: ./chuc2010 --export-wav
// =====================================================================
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#endif

// ----------------------- CẤU HÌNH: sửa tại đây -----------------------
static const std::string CLASS_NAME = "11A1";
static const std::string SENDER     = "Một người bạn cùng lớp"; // đổi thành tên bạn

static const std::vector<std::string> CARD = {
    "Gửi các bạn nữ 11A1 thân yêu,",
    "",
    "Nhân ngày 20/10, tớ chỉ mong các bạn luôn được là chính mình:",
    "rạng rỡ như nắng sớm, mạnh mẽ như cơn gió lớn",
    "và dịu dàng như những cánh hoa đầu mùa.",
    "",
    "Mong mọi áp lực bài vở hóa thành điểm cao,",
    "mọi ước mơ đều tìm được lối đi,",
    "và mỗi ngày đến lớp đều có một tràng cười thật to.",
    "",
    "Cảm ơn các bạn đã khiến 11A1 trở thành một tập thể",
    "đáng nhớ, ấm áp và rất đỗi đáng yêu.",
    "",
    "Chúc các bạn một ngày 20/10 thật rực rỡ, ngọt ngào",
    "và ngập tràn quà, hoa, tiếng cười! ✿",
};

static const std::vector<std::string> WISHES = {
    "Luôn xinh đẹp ✿",      "Bình yên, nhẹ nhàng",  "Nụ cười rạng rỡ ✦",
    "Được yêu thương ♥",    "Thật hạnh phúc",       "Điểm 10 nhé ★",
    "Mạnh mẽ & tự tin",     "Mọi điều như ý",       "Đẹp như hoa ❀",
    "Học giỏi, chơi vui",   "Tỏa sáng nhé ✧",       "Ngọt ngào như kẹo ♡",
};
// ---------------------------------------------------------------------

static double SPEED = 1.0;  // --fast => 0.05
static const int W = 72, H = 24;
static std::mt19937 rng{std::random_device{}()};

static int rnd(int a, int b) { return std::uniform_int_distribution<int>(a, b)(rng); }
static double rndf(double a, double b) { return std::uniform_real_distribution<double>(a, b)(rng); }

static void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(ms * SPEED)));
}

static void setup_console() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) SetConsoleMode(h, mode | 0x0004);  // VT processing
#endif
}

// --------------------------- tiện ích UTF-8 ---------------------------
static std::vector<std::string> utf8_split(const std::string& s) {
    std::vector<std::string> out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        size_t n = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : 4;
        out.push_back(s.substr(i, n));
        i += n;
    }
    return out;
}
static int utf8_len(const std::string& s) { return (int)utf8_split(s).size(); }

// ------------------------------ Âm thanh -------------------------------
// Nhạc nền (hộp nhạc + hợp âm rải) và tiếng pháo hoa được TỰ TẠO bằng code,
// ghi ra file .wav tạm rồi phát bằng trình phát có sẵn của hệ điều hành.
static bool SOUND = true;
static const int SR = 22050;
static const double PI = 3.14159265358979323846;
static std::string BGM_PATH, POP_PATH;

static void add_note(std::vector<float>& mix, double start, double dur, double midi,
                     double vol, double decay) {
    double f = 440.0 * std::pow(2.0, (midi - 69.0) / 12.0);
    size_t s0 = (size_t)(start * SR), n = (size_t)((dur + 1.2) * SR);  // +1.2s đuôi ngân
    if (mix.size() < s0 + n) mix.resize(s0 + n, 0.f);
    for (size_t i = 0; i < n; ++i) {
        double t = (double)i / SR;
        double env = std::min(1.0, t / 0.005) * std::exp(-t * decay);
        double w = std::sin(2 * PI * f * t) +
                   0.35 * std::sin(2 * PI * 2 * f * t) * std::exp(-t * 4) +
                   0.12 * std::sin(2 * PI * 3 * f * t) * std::exp(-t * 6);
        mix[s0 + i] += (float)(vol * env * w);
    }
}

static void write_wav(const std::string& path, std::vector<float> mix) {
    float peak = 0.001f;
    for (float v : mix) peak = std::max(peak, std::fabs(v));
    std::ofstream f(path, std::ios::binary);
    auto u32 = [&](uint32_t v) { for (int i = 0; i < 4; ++i) f.put((char)((v >> (8 * i)) & 0xFF)); };
    auto u16 = [&](uint16_t v) { for (int i = 0; i < 2; ++i) f.put((char)((v >> (8 * i)) & 0xFF)); };
    uint32_t bytes = (uint32_t)(mix.size() * 2);
    f.write("RIFF", 4); u32(36 + bytes); f.write("WAVEfmt ", 8);
    u32(16); u16(1); u16(1); u32(SR); u32(SR * 2); u16(2); u16(16);
    f.write("data", 4); u32(bytes);
    for (float v : mix) u16((uint16_t)(int16_t)std::lround(v / peak * 0.85 * 32767));
}

// Giai điệu tự sáng tác (Đô trưởng, 96 BPM, ~42 giây): C - Am - F - G
static std::vector<float> make_bgm() {
    struct MN { int m; double b; };
    static const int CH[4][3] = {{60, 64, 67}, {57, 60, 64}, {53, 57, 60}, {55, 59, 62}};
    static const std::vector<std::vector<MN>> A = {
        {{76, 1}, {79, 1}, {84, 2}},          {{81, 1.5}, {79, 0.5}, {76, 2}},
        {{77, 1}, {81, 1}, {84, 1}, {81, 1}}, {{79, 3}, {0, 1}}};
    static const std::vector<std::vector<MN>> B = {
        {{84, 1}, {83, 0.5}, {81, 0.5}, {79, 2}}, {{76, 1}, {81, 1}, {84, 2}},
        {{81, 1}, {79, 1}, {77, 1}, {76, 1}},     {{74, 2}, {79, 2}}};
    const double beat = 60.0 / 96.0;
    std::vector<float> mix;
    double t = 0;
    auto bar = [&](int c, const std::vector<MN>& mel) {
        add_note(mix, t, 4 * beat, CH[c][0] - 12, 0.35, 1.2);  // âm trầm
        static const int pat[8] = {0, 1, 2, 1, 2, 1, 2, 1};
        for (int k = 0; k < 8; ++k)                              // hợp âm rải
            add_note(mix, t + k * beat / 2, beat / 2, CH[c][pat[k]], 0.14, 3.0);
        double mt = t;
        for (auto& n : mel) {                                    // giai điệu
            if (n.m) add_note(mix, mt, n.b * beat, n.m, 0.32, 2.2);
            mt += n.b * beat;
        }
        t += 4 * beat;
    };
    for (int rep = 0; rep < 2; ++rep) {
        for (int c = 0; c < 4; ++c) bar(c, A[c]);
        for (int c = 0; c < 4; ++c) bar(c, B[c]);
    }
    bar(0, {{84, 4}});  // kết bằng hợp âm Đô
    return mix;
}

// Tiếng pháo: "bùm" trầm + lách tách
static std::vector<float> make_pop() {
    std::mt19937 g(7);
    std::uniform_real_distribution<double> u(-1.0, 1.0);
    std::vector<float> mix((size_t)(1.2 * SR), 0.f);
    double lp = 0;
    for (size_t i = 0; i < mix.size(); ++i) {
        double t = (double)i / SR;
        lp += 0.15 * (u(g) - lp);
        mix[i] = (float)(std::sin(2 * PI * (55 + 90 * std::exp(-t * 14)) * t) * std::exp(-t * 6) * 0.9 +
                         lp * std::exp(-t * 9) * 1.6);
    }
    for (int c = 0; c < 28; ++c) {  // tiếng lách tách
        size_t s0 = (size_t)(rndf(0.12, 0.9) * SR);
        for (size_t i = 0; i < (size_t)(0.012 * SR) && s0 + i < mix.size(); ++i)
            mix[s0 + i] += (float)(u(g) * std::exp(-(double)i / (0.003 * SR)) * 0.25);
    }
    return mix;
}

static std::string temp_dir() {
#ifdef _WIN32
    char b[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, b);
    return std::string(b, n);
#else
    return "/tmp/";
#endif
}

static void sh(const std::string& cmd) { int r = std::system(cmd.c_str()); (void)r; }

static void play_file_bg(const std::string& path) {  // phát nền trên Linux/macOS
#ifdef __APPLE__
    sh("afplay '" + path + "' >/dev/null 2>&1 &");
#else
    sh(("if command -v paplay >/dev/null 2>&1; then paplay '" + path +
                 "'; elif command -v aplay >/dev/null 2>&1; then aplay -q '" + path +
                 "'; else ffplay -nodisp -autoexit -loglevel quiet '" + path +
                 "'; fi >/dev/null 2>&1 &"));
#endif
}

static void sound_init() {
    if (!SOUND) return;
    BGM_PATH = temp_dir() + "chuc2010_bgm.wav";
    POP_PATH = temp_dir() + "chuc2010_pop.wav";
    write_wav(BGM_PATH, make_bgm());
    write_wav(POP_PATH, make_pop());
}
static void play_music() {
    if (!SOUND) return;
#ifdef _WIN32
    mciSendStringA(("open \"" + BGM_PATH + "\" type waveaudio alias bgm").c_str(), 0, 0, 0);
    mciSendStringA("play bgm", 0, 0, 0);
#else
    play_file_bg(BGM_PATH);
#endif
}
static void play_pop() {
    if (!SOUND) return;
#ifdef _WIN32
    PlaySoundA(POP_PATH.c_str(), NULL, SND_FILENAME | SND_ASYNC);
#else
    play_file_bg(POP_PATH);
#endif
}
static void sound_shutdown() {
    if (!SOUND) return;
#ifdef _WIN32
    mciSendStringA("close bgm", 0, 0, 0);
    PlaySoundA(NULL, NULL, 0);
#else
    sh("pkill -f 'chuc2010_[b]gm.wav' >/dev/null 2>&1; pkill -f 'chuc2010_[p]op.wav' >/dev/null 2>&1");
#endif
    std::remove(BGM_PATH.c_str());
    std::remove(POP_PATH.c_str());
}

// ------------------------------ màu sắc -------------------------------
static std::string rgb(int r, int g, int b) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}
static std::string rainbow(double h) {  // h: độ, 0..360
    h = std::fmod(h, 360.0);
    if (h < 0) h += 360;
    double s = 0.55, v = 1.0;  // pastel cho dịu mắt
    double c = v * s, x = c * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1)), m = v - c;
    double r = 0, g = 0, b = 0;
    if (h < 60) { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else { r = c; b = x; }
    return rgb((int)((r + m) * 255), (int)((g + m) * 255), (int)((b + m) * 255));
}
static const char* RESET = "\033[0m";

static void clear_screen() { std::cout << "\033[2J\033[H"; }
static void goto_xy(int x, int y) { std::cout << "\033[" << y << ";" << x << "H"; }

// ------------------------------ Màn 1: chữ 20·10 ----------------------
static void banner() {
    static const char* F2[5] = {"█████", "    █", "█████", "█    ", "█████"};
    static const char* F0[5] = {"█████", "█   █", "█   █", "█   █", "█████"};
    static const char* F1[5] = {"  █  ", " ██  ", "  █  ", "  █  ", "█████"};
    static const char* FD[5] = {"   ", "   ", " █ ", "   ", "   "};
    const char** glyphs[] = {F2, F0, FD, F1, F0};
    clear_screen();
    std::cout << "\n\n";
    std::string pad(22, ' ');
    for (int r = 0; r < 5; ++r) {
        std::cout << pad;
        int col = 0;
        for (auto g : glyphs) {
            for (auto& ch : utf8_split(g[r])) {
                std::cout << rainbow(col * 9 + r * 20) << ch;
                ++col;
            }
            std::cout << "  ";
            col += 2;
        }
        std::cout << RESET << "\n";
        sleep_ms(250);
    }
    std::cout << "\n" << std::string(20, ' ') << rgb(255, 214, 140)
              << "✿  C H Ú C   M Ử N G   N G À Y   P H Ụ   N Ữ  ✿" << RESET << "\n";
    sleep_ms(1200);
}

// ------------------------------ Màn 2: mưa tim ------------------------
struct Cell { std::string ch = " "; std::string color; };
struct Particle { double x, y, vy; std::string ch; double hue; };
struct Wish { std::string text; int x, y; double hue; };

static void put_text(std::vector<std::vector<Cell>>& buf, int x, int y,
                     const std::string& text, double hue, double hue_step) {
    int i = 0;
    for (auto& ch : utf8_split(text)) {
        int cx = x + i;
        if (y >= 0 && y < H && cx >= 0 && cx < W) buf[y][cx] = {ch, rainbow(hue + i * hue_step)};
        ++i;
    }
}

static void heart_rain(int frames) {
    static const char* GL[] = {"♥", "♡", "✦", "✧", "❀", "✿", "·", "★", "•"};
    std::vector<Particle> ps;
    for (int i = 0; i < 70; ++i)
        ps.push_back({rndf(0, W - 1), rndf(0, H - 1), rndf(0.15, 0.6), GL[rnd(0, 8)], rndf(0, 360)});

    std::vector<Wish> ws;
    for (size_t i = 0; i < WISHES.size(); ++i) {
        int col = (i % 2 == 0) ? rnd(1, 14) : rnd(44, 54);
        int row = 6 + (int)(i / 2) * 3 + rnd(0, 1);
        ws.push_back({WISHES[i], col, row, rndf(0, 360)});
    }

    clear_screen();
    std::cout << "\033[?25l";
    for (int f = 0; f < frames; ++f) {
        std::vector<std::vector<Cell>> buf(H, std::vector<Cell>(W));
        for (auto& p : ps) {
            p.y += p.vy;
            if (p.y >= H) { p.y = 0; p.x = rndf(0, W - 1); }
            int px = (int)p.x, py = (int)p.y;
            buf[py][px] = {p.ch, rainbow(p.hue + f * 3)};
        }
        // tiêu đề
        std::string t1 = "✿  CHÚC MỪNG NGÀY 20·10  ✿";
        std::string t2 = "PHỤ NỮ VIỆT NAM - " + CLASS_NAME + " ♥";
        put_text(buf, (W - utf8_len(t1)) / 2, 1, t1, f * 6, 12);
        put_text(buf, (W - utf8_len(t2)) / 2, 3, t2, 40 + f * 4, 8);
        // lời chúc nhấp nháy xuất hiện dần
        for (size_t i = 0; i < ws.size(); ++i) {
            if (f < (int)i * 4) continue;
            if ((f / 5 + (int)i) % 7 == 0) continue;  // nhấp nháy
            put_text(buf, ws[i].x, ws[i].y, ws[i].text, ws[i].hue + f * 4, 5);
        }
        std::string out = "\033[H";
        for (auto& row : buf) {
            for (auto& c : row) out += c.color + c.ch;
            out += "\033[0m\n";
        }
        std::cout << out << std::flush;
        sleep_ms(70);
    }
}

// ------------------------------ Màn 3: trái tim -----------------------
static void draw_heart() {
    clear_screen();
    std::cout << "\n";
    for (double y = 1.35; y >= -1.25; y -= 0.14) {
        std::cout << std::string(26, ' ');
        for (double x = -1.45; x <= 1.45; x += 0.07) {
            double a = x * x + y * y - 1;
            bool in = a * a * a - x * x * y * y * y <= 0;
            if (in) {
                double t = (1.35 - y) / 2.6;  // 0 trên, 1 dưới
                std::cout << rgb(255, (int)(120 - t * 90), (int)(160 - t * 80)) << "♥";
            } else {
                std::cout << " ";
            }
        }
        std::cout << RESET << "\n";
        sleep_ms(80);
    }
    sleep_ms(500);
}

// ------------------------------ Màn 4: thẻ chúc ----------------------
static void show_card(bool typewriter) {
    clear_screen();
    int inner = 0;
    for (auto& l : CARD) inner = std::max(inner, utf8_len(l));
    inner += 2;
    std::string ind(std::max(0, (80 - inner - 2) / 2), ' ');
    auto line = [&](int n) { std::string s; for (int i = 0; i < n; ++i) s += "─"; return s; };

    std::cout << "\n" << ind << rgb(255, 170, 200) << "╭" << line(inner) << "╮\n";
    int k = 0;
    for (auto& l : CARD) {
        std::cout << ind << rgb(255, 170, 200) << "│ ";
        for (auto& ch : utf8_split(l)) {
            std::cout << rainbow(k * 7) << ch << std::flush;
            ++k;
            if (typewriter && ch != " ") sleep_ms(28);
        }
        std::cout << std::string(inner - 1 - utf8_len(l), ' ') << rgb(255, 170, 200) << "│\n";
    }
    std::string sign = "— " + SENDER + " ♥";
    std::cout << ind << rgb(255, 170, 200) << "│" << std::string(inner - utf8_len(sign) - 1, ' ')
              << rgb(255, 214, 140) << sign << " " << rgb(255, 170, 200) << "│\n";
    std::cout << ind << "╰" << line(inner) << "╯" << RESET << "\n";
}

// ------------------------------ Pháo hoa -----------------------------
static void fireworks(int bursts) {
    clear_screen();
    static const char* SP[] = {"✦", "✧", "★", "•", "*", "♥"};
    for (int b = 0; b < bursts; ++b) {
        int cx = rnd(14, W - 14), cy = rnd(6, H - 6);
        double hue = rndf(0, 360);
        const char* g = SP[rnd(0, 5)];
        play_pop();
        const int N = 30, STEPS = 9;
        auto plot = [&](int step, bool erase) {
            for (int i = 0; i < N; ++i) {
                double ang = 6.2831853 * i / N;
                int x = cx + (int)std::lround(std::cos(ang) * step * 2.2);
                int y = cy + (int)std::lround(std::sin(ang) * step);
                if (x < 1 || x > W || y < 2 || y > H) continue;
                goto_xy(x, y);
                if (erase) std::cout << " ";
                else std::cout << rainbow(hue + i * 6) << g;
            }
        };
        for (int s = 1; s <= STEPS; ++s) {
            if (s > 1) plot(s - 1, true);
            plot(s, false);
            std::cout << std::flush;
            sleep_ms(70);
        }
        sleep_ms(250);
        plot(STEPS, true);
        goto_xy(14, 1);
        std::cout << rainbow(b * 50) << "✿ Chúc mừng 20/10 - các bạn nữ " << CLASS_NAME << " ✿" << RESET << std::flush;
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--fast") SPEED = 0.05;
        else if (a == "--mute") SOUND = false;
        else if (a == "--export-wav") {
            write_wav("nhac_nen.wav", make_bgm());
            write_wav("phao_hoa.wav", make_pop());
            std::cout << "Đã xuất nhac_nen.wav và phao_hoa.wav\n";
            return 0;
        }
    }
    setup_console();
    std::ios::sync_with_stdio(false);
    sound_init();
    play_music();

    banner();
    heart_rain(90);
    draw_heart();
    show_card(true);

    std::cout << "\033[?25h\n  " << rgb(255, 214, 140)
              << "Nhấn Enter để bắn pháo hoa ✦  (gõ q rồi Enter để thoát)" << RESET << "\n  ";
    std::string in;
    while (std::getline(std::cin, in)) {
        if (!in.empty() && (in[0] == 'q' || in[0] == 'Q')) break;
        std::cout << "\033[?25l";
        fireworks(5);
        show_card(false);
        std::cout << "\033[?25h\n  " << rgb(255, 214, 140)
                  << "Enter: bắn pháo hoa tiếp  |  q: thoát" << RESET << "\n  ";
    }
    sound_shutdown();
    std::cout << "\033[?25h" << RESET << "\n  Chúc " << CLASS_NAME << " một ngày 20/10 thật vui! ♥\n";
    return 0;
}
